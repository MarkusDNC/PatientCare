using System;
using System.IO;
using Gtk;

public partial class MainWindow: Gtk.Window
{

	public MainWindow () : base (Gtk.WindowType.Toplevel)
	{
		Build ();

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

		protected void OnEntryDoctorChanged (object sender, EventArgs e)
	{
		labelDoctor.Text = entryDoctor.Text;
	}

	protected void OnEntryPatientChanged (object sender, EventArgs e)
	{
		labelPatient.Text = entryPatient.Text;
	}

	protected void OnAddDoctorClicked (object sender, EventArgs e)
	{
		if (!Directory.Exists("../../Doctors/" + entryDoctor.Text)) 
		{
			// Try to create the directory.
			DirectoryInfo di = Directory.CreateDirectory("../../Doctors/" + entryDoctor.Text);
			Console.WriteLine("The directory was created successfully at {0}.", Directory.GetCreationTime("Doctors/" + entryDoctor.Text));

		}
		PatientCare.Program.writeImage ("../../Doctors/" + entryDoctor.Text + "/" + entryDoctor.Text);
		listDoctors.AppendText (entryDoctor.Text);
		labelDoctor.Text = "Enter doctor name";
	}

	protected void OnAddPatientClicked (object sender, EventArgs e)
	{
		if (!Directory.Exists("../../Patients/" + entryPatient.Text)) 
		{
			// Try to create the directory.
			DirectoryInfo di = Directory.CreateDirectory("../../Patients/" + entryPatient.Text);
			Console.WriteLine("The directory was created successfully at {0}.", Directory.GetCreationTime("Doctors/" + entryDoctor.Text));

		}
		PatientCare.Program.writeImage ("../../Patients/" + entryPatient.Text + "/" + entryPatient.Text);
		listPatients.AppendText (entryPatient.Text);
		labelPatient.Text = "Enter patient name";
	}
		
}