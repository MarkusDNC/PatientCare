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
using SourceAFIS.Simple; // import namespace SourceAFIS.Simple

namespace Sample
{


    class Program
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

        // Take fingerprint image file and create Person object from the image
		static MyPerson Enroll(byte [,] image, string name)
        {
            Console.WriteLine("Enrolling {0}...", name);

            // Initialize empty fingerprint object and set properties
            MyFingerprint fp = new MyFingerprint();
            //fp.Filename = filename;
            // Load image from the file
            //Console.WriteLine(" Loading image from {0}...", filename);

			//Egen modifiering
            //BitmapImage image = new BitmapImage(new Uri(filename, UriKind.RelativeOrAbsolute));
			fp.Image = image;
            //fp.AsBitmapSource = image;
            // Above update of fp.AsBitmapSource initialized also raw image in fp.Image
            // Check raw image dimensions, Y axis is first, X axis is second
            //Console.WriteLine(" Image size = {0} x {1} (width x height)", fp.Image.GetLength(1), fp.Image.GetLength(0));

            // Initialize empty person object and set its properties
            MyPerson person = new MyPerson();
            person.Name = name;
            // Add fingerprint to the person
            person.Fingerprints.Add(fp);

            // Execute extraction in order to initialize fp.Template
            //Console.WriteLine(" Extracting template...");
            Afis.Extract(person);
            // Check template size
            //Console.WriteLine(" Template size = {0} bytes", fp.Template.Length);

            return person;
        }

        static void Main(string[] args)
        {
            // Initialize SourceAFIS
			for(int i=0; i<100; i++){
				Program.doctor_array [i] = "../empty.raw";
				Program.patient_array [i] = "../empty.raw";
				if (i > 50) {
					Program.doctor_array[i] = "../markus1.raw";
				}
				if (i > 80) {
					Program.patient_array[i] = "../oskar.raw";
				}
			}
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

			// Initialize path to images
            Afis = new AfisEngine();

            // Enroll some people
            List<MyPerson> db_doctor = new List<MyPerson>();
			List<MyPerson> db_patient = new List<MyPerson>();

			db_doctor.Add (Enroll (readImage ("../../markus1.raw"), "Markus "));
			//db_patient.Add (Enroll (readImage (ImagePath, "../oskar.raw"), "Oskar"));

            // Save the database to disk and load it back, just to try out the serialization
            /*BinaryFormatter formatter = new BinaryFormatter();
            Console.WriteLine("Saving database...");
            using (Stream stream = File.Open("database.dat", FileMode.Create))
                formatter.Serialize(stream, database);
            Console.WriteLine("Reloading database...");
            using (FileStream stream = File.OpenRead("database.dat"))
                database = (List<MyPerson>)formatter.Deserialize(stream);*/

			Tuple <byte[,], byte[,]> images;
			MyPerson doctorMatch = null;
			MyPerson patientMatch = null;
			MyPerson doctorProbe;
			MyPerson patientProbe;
			Afis.Threshold = 30;
			while(doctorMatch == null ){//|| patientMatch == null){
				images = captureImages ();
				doctorProbe = Enroll (images.Item1, "#probe1");
				//patientProbe = Enroll (images.Item2, "#probe2");

				doctorMatch = Afis.Identify (doctorProbe, db_doctor).FirstOrDefault () as MyPerson;
				//patientMatch = Afis.Identify (patientProbe, db_patient).FirstOrDefault () as MyPerson;
			}

			Console.WriteLine ("Welcom Dr Markus");

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
				/*
				FileStream output = new FileStream ("image2.raw", FileMode.OpenOrCreate, FileAccess.Write);
				foreach (byte row in result)
					output.WriteByte (row);
				output.Close ();*/

				}
			return result;
			}
		}
}
