using System;
using System.IO;
using Gtk;
using PatientCare;
using MySql;
using System.Threading;
public partial class MainWindow: Gtk.Window
{

	Controller program;

	public MainWindow (Controller program) : base (Gtk.WindowType.Toplevel)
	{
		Build ();

		this.program = program;

		scan.Child.ModifyFont(Pango.FontDescription.FromString("Arial 18"));

		var dirDoctors = Directory.GetDirectories("../../Doctors");
		for (int i = 0; i < dirDoctors.Length; i++) {
			String temp = dirDoctors [i].Replace ("../../Doctors/", "");
			listDoctors.AppendText(temp);
		}

		var dirPatients = Directory.GetDirectories("../../Patients");
		for (int i = 0; i < dirPatients.Length; i++) {
			String temp = dirPatients [i].Replace ("../../Patients/", "");
			listPatients.AppendText(temp);
		}

	}
		
	protected void OnDeleteEvent (object sender, DeleteEventArgs a)
	{
		Application.Quit ();
		a.RetVal = true;
	}

	protected void OnAddDoctorClicked (object sender, EventArgs e)
	{
		if (!Directory.Exists("../../Doctors/" + entryDoctor.Text)) 
		{
			// Try to create the directory.
			Directory.CreateDirectory("../../Doctors/" + entryDoctor.Text);
			Console.WriteLine("The directory was created successfully at {0}.", Directory.GetCreationTime("Doctors/" + entryDoctor.Text));
			listDoctors.AppendText (entryDoctor.Text);
		}
		PatientCare.Controller.writeImage ("../../Doctors/" + entryDoctor.Text + "/" + entryDoctor.Text);

		program.addDoctor ("../../Doctors/" + entryDoctor.Text + "/" + entryDoctor.Text, entryDoctor.Text);
		entryDoctor.Text = "Enter doctor name";
	}

	protected void OnAddPatientClicked (object sender, EventArgs e)
	{
		if (!Directory.Exists("../../Patients/" + entryPatient.Text)) 
		{
			// Try to create the directory.
			Directory.CreateDirectory("../../Patients/" + entryPatient.Text);
			Console.WriteLine("The directory was created successfully at {0}.", Directory.GetCreationTime("Doctors/" + entryDoctor.Text));
			listPatients.AppendText (entryPatient.Text);
		}
		PatientCare.Controller.writeImage ("../../Patients/" + entryPatient.Text + "/" + entryPatient.Text);
		program.addPatient ("../../Patients/" + entryPatient.Text + "/" + entryPatient.Text, entryPatient.Text);
		entryPatient.Text = "Please enter patient name";
	}
		
	protected void OnScanClicked (object sender, EventArgs e)
	{
		scan.Sensitive = false;
		(new Thread(() => {

			Gtk.Application.Invoke(delegate {
				result.Text = "Scanning Doctor";
				result.Show ();
			});

			Boolean verified = this.program.verifyDoctor (listDoctors.ActiveText);

			Gtk.Application.Invoke(delegate {
				
				result.Text = verified ? "\"Welcome " + listDoctors.ActiveText + " , now scanning patient...\"" : "VERIFICATION FAILED!";
			});

			if(!verified) {
				Gtk.Application.Invoke(delegate {
					scan.Sensitive = true;
				});
				Thread.CurrentThread.Abort();
			}

		
			verified = this.program.verifyPatient (listPatients.ActiveText);

			if (verified)
			{
				var process = System.Diagnostics.Process.Start("raspistill", "-o cam.jpg");
				process.WaitForExit();
				patientImage.Pixbuf = new Gdk.Pixbuf("cam.jpg");
			}

			Gtk.Application.Invoke(delegate {
				result.Text = verified ? "VERIFICATION SUCCESSFUL!" : "VERIFICATION FAILED!";
				scan.Sensitive = true;
			});

			Thread.CurrentThread.Abort();

		})).Start();

	}

	public void doctorScanned(){
		result.Text = "Welcom doctor, now scanning patient...";
	}

	protected void OnButton1Clicked (object sender, EventArgs e)
	{
		Console.WriteLine ("../../Patients/" + listPatients.ActiveText + "/" + listPatients.ActiveText + ".raw");
		if (Directory.Exists ("../../Patients/" + listPatients.ActiveText)) {
			File.Delete ("../../Patients/" + listPatients.ActiveText + "/" + listPatients.ActiveText + ".raw");
			Directory.Delete ("../../Patients/" + listPatients.ActiveText +"/");
			program.removePatient (listPatients.ActiveText);
			listPatients.RemoveText (listPatients.Active);
		}
	}

	protected void OnRemoveDoctorClicked (object sender, EventArgs e)
	{
		Console.WriteLine ("../../Doctors/" + listDoctors.ActiveText + "/" + listDoctors.ActiveText + ".raw");
		if (Directory.Exists ("../../Doctors/" + listDoctors.ActiveText)) {
			File.Delete ("../../Doctors/" + listDoctors.ActiveText + "/" + listDoctors.ActiveText + ".raw");
			Directory.Delete ("../../Doctors/" + listDoctors.ActiveText +"/");
			program.removeDoctor (listDoctors.ActiveText);
			listDoctors.RemoveText (listDoctors.Active);
		}
	}
}