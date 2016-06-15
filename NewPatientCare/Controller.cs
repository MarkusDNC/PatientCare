using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using System.Linq;
using System.Linq.Expressions;
using System.Windows.Media.Imaging;
using System.IO;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;
using System.Media;
using SourceAFIS.Simple;
using Gtk;

namespace PatientCare
{


	public class Controller
	{
		[DllImport("libftdi")]
		public static extern int ftdi_get_library_version ();
		[DllImport("libfingerprint")]
		public static extern int openDevice();
		[DllImport("libfingerprint")]
		public static extern int resetSensor ();
		[DllImport("libfingerprint")]
		public static extern int captureImage(byte adcref, byte drivc, byte sensemode, byte[] pdata);

		static string[] doctor_array = new string[100];
		static string[] patient_array = new string[100];
		// Inherit from Fingerprint in order to add Filename field
		[Serializable]
		class MyFingerprint : Fingerprint
		{
			public string Filename;
		}

		// Inherit from Person in order to add Name field
		[Serializable]
		class MyPerson : Person
		{
			public string Name;
		}


		static readonly string ImagePath = Path.Combine(Path.Combine("..", ".."), "images");

		// Shared AfisEngine instance (cannot be shared between different threads though)
		static AfisEngine Afis;

		List<MyPerson> db_doctor;
		List<MyPerson> db_patient;

		public Controller(){

			Afis = new AfisEngine ();
			this.db_doctor = new List<MyPerson>();
			this.db_patient = new List<MyPerson>();

		}

		public void Initialize(){
		
			var dirDoctors = Directory.GetDirectories("../../Doctors");
			for (int i = 0; i < dirDoctors.Length; i++) {
				String name = dirDoctors [i].Replace ("../../Doctors/", "");
				var imageFiles = Directory.GetFiles (dirDoctors [i]);
				foreach(String imagefile in imageFiles) {
					this.db_doctor.Add (Enroll (readImage (imagefile), name));
				}
			}

			var dirPatients = Directory.GetDirectories("../../Patients");
			for (int i = 0; i < dirPatients.Length; i++) {
				String name = dirPatients [i].Replace ("../../Patients/", "");
				var imageFiles = Directory.GetFiles (dirPatients [i]);
				foreach(String imagefile in imageFiles) {
					this.db_patient.Add (Enroll (readImage (imagefile), name));
				}
			}
		}


		public void addDoctor(String path, String name){
			this.db_doctor.Add (Enroll (readImage (path + ".raw"), name));
		}

		public void addPatient(String path, String name){
			this.db_patient.Add (Enroll (readImage (path + ".raw"), name));
		}

		public Boolean verifyDoctor( String drName ){

			Tuple <byte[,], byte[,]> images;
			MyPerson doctorMatch = null;
			MyPerson doctorProbe;
			Afis.Threshold = 30;

			MyPerson doctor = null;

			foreach (MyPerson person in this.db_doctor) {

				if (person.Name == drName) {
					doctor = person;
					break;
				}
			}
			double currentTime = GetCurrentMilli ();	

			while(doctorMatch == null && GetCurrentMilli() < (currentTime + 20000) ){//|| patientMatch == null){
				images = captureImages ();
				doctorProbe = Enroll (images.Item1, "#probe1");
				float value = Afis.Verify (doctorProbe, doctor);
				if (value > 0.8) {
					doctorMatch = doctorProbe;
				}
			}
			if (doctorMatch != null) {
				Console.WriteLine ("Welcome Dr " + doctor.Name);
				SystemSounds.Exclamation.Play ();
			} 
			else {
				return false;
			}
			return true;
		}

		public Boolean verifyPatient(String name){

			Tuple <byte[,], byte[,]> images;
			MyPerson patientMatch = null;
			MyPerson patientProbe;
			MyPerson patient = null;

			foreach (MyPerson person in this.db_patient) {
				if (person.Name == name) {
					patient = person;
					break;
				}
			}

			double currentTime = GetCurrentMilli ();
			while(patientMatch == null && GetCurrentMilli() < (currentTime + 20000) ){//|| patientMatch == null){
				images = captureImages ();
				patientProbe = Enroll (images.Item1, "#patientProbe");

				float value = Afis.Verify (patientProbe, patient);
				if (value > 0.8) {
					patientMatch = patientProbe;
				}
			}

			if (patientMatch != null) {
				Console.WriteLine ("Welcome " + patient.Name);
				SystemSounds.Asterisk.Play ();
			} else {

				return false;
			}
			return true;

		}

		static MyPerson Enroll(byte [,] image, string name)
		{
			Console.WriteLine("Enrolling {0}...", name);

			MyFingerprint fp = new MyFingerprint();

			fp.Image = image;

			MyPerson person = new MyPerson();
			person.Name = name;
			person.Fingerprints.Add(fp);

			Afis.Extract(person);

			return person;
		}
			

		static Tuple<byte[,], byte[,]> captureImages(){
			//Tuple <string,string> imageList = new Tuple<string,string>(doctor_array [n],patient_array [n] );

			// TODO: Read data from sensor, convert to 2d array, return reading from both sensorns
			byte[] img = new byte[152*200];
			captureImage (2, 127, 0, img);
			byte[,] fingerprintImage = convertImage (img);
			return new Tuple<byte[,], byte[,]>(fingerprintImage,new byte[1,1]);
			//return imageList;
		}

		static byte[,] convertImage(byte[] image){
			byte[,] result = new byte[200, 152];

			for (int i = 0; i < 200; i++) {
				for (int j = 0; j < 152; j++) {
					result [i, j] = image [i *152 + j];
				}
			}
			return result;
		}

		static byte[] GetRawImage(String filename){
			byte[] bt;
			FileStream fs = new FileStream (filename, FileMode.Open, FileAccess.Read);

			BinaryReader br = new BinaryReader (fs);
			bt = br.ReadBytes ((int)br.BaseStream.Length);
			br.Close();
			return bt;
		}

		public static void writeImage(String filename){

			byte[] img = new byte[152*200];
			captureImage (2, 127, 0, img);
			using (FileStream fs = File.Create (filename + ".raw")) {
				fs.Write (img, 0, img.Length);
			}
		}


		static byte[,] readImage(String filepath){

			byte[][] rawfile = new byte[200][];

			for(int j=0; j<200; j++){
				rawfile [j] = new byte[152];
			}

			byte [,] result = new byte[200,152];
			using (FileStream
				FileStream = new FileStream(filepath, FileMode.Open, FileAccess.Read)){
				for(int i=0; i<200; i++){
					int read = 0;
					while (read != 152) {
						read += FileStream.Read (rawfile[i], read, 152);
					}
				}

				for(int i=0; i<200; i++){
					for(int j=0; j<152; j++){
						result[i,j] = rawfile[i][j];

					}
				}
			}
			return result;
		}

		public static double GetCurrentMilli(){
	        DateTime Jan1970 = new DateTime(1970, 1, 1, 0, 0,0,DateTimeKind.Utc);
	        TimeSpan javaSpan = DateTime.UtcNow - Jan1970;
	        return javaSpan.TotalMilliseconds;
	    }

		static void Main(string[] args)
		{

			// Initialize SourceAFIS
			try{
				ftdi_get_library_version ();
			}
			catch(EntryPointNotFoundException){
				Console.WriteLine ("Loaded library");
			}
			int status = openDevice();
			Console.WriteLine (status);

			if (status < 0) {
				Console.WriteLine ("Could not open device!");
			}

			if (resetSensor() != 0) {
				Console.WriteLine ("Could not reset sensor");
				return;
			} else {
				Console.WriteLine ("Sensor reset");
			}

			Controller program = new Controller ();
			program.Initialize ();
			Application.Init ();
			MainWindow win = new MainWindow (program);
			win.Show ();
			Application.Run ();

		}
	
	}
}
