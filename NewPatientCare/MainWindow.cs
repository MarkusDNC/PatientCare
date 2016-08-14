using System;
using System.IO;
using Gtk;
using PatientCare;
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
			listDoctors.AppendText (entryDoctor.Text);
		}
		PatientCare.Controller.writeImage ("../../Doctors/" + entryDoctor.Text + "/" + entryDoctor.Text);

		labelDoctor.Text = "Doctor name";

		program.addDoctor ("../../Doctors/" + entryDoctor.Text + "/" + entryDoctor.Text, entryDoctor.Text);
		entryDoctor.Text = "Enter doctor name";
	}

	protected void OnAddPatientClicked (object sender, EventArgs e)
	{
		if (!Directory.Exists("../../Patients/" + entryPatient.Text)) 
		{
			// Try to create the directory.
			DirectoryInfo di = Directory.CreateDirectory("../../Patients/" + entryPatient.Text);
			Console.WriteLine("The directory was created successfully at {0}.", Directory.GetCreationTime("Doctors/" + entryDoctor.Text));
			listPatients.AppendText (entryPatient.Text);
		}
		PatientCare.Controller.writeImage ("../../Patients/" + entryPatient.Text + "/" + entryPatient.Text);
		labelPatient.Text = "Patient name";
		program.addPatient ("../../Patients/" + entryPatient.Text + "/" + entryPatient.Text, entryPatient.Text);
		entryPatient.Text = "Please enter patient name";
	}
		
	protected void OnScanClicked (object sender, EventArgs e)
	{

		result.Text = "Scanning Doctor";
		result.Show ();
		Boolean verified = this.program.verifyDoctor (listDoctors.ActiveText);

		if (verified) {
			result.Text = "\"Welcome doctor, scanning patient...\"";

		} else {
			result.Text = "VERIFICATION FAILED!";
			return;
		}

	
		verified = this.program.verifyPatient (listPatients.ActiveText);

		if (verified) {
			result.Text = "VERIFICATION SUCCESSFUL!";

		} else {
			result.Text = "VERIFICATION FAILED!";
			return;
		}

	}

	public void doctorScanned(){
		result.Text = "Welcom doctor, scanning patient...";
	}

	protected void OnScanPressed (object sender, EventArgs e)
	{
		result.Text = "Scanning doctor/patient...";
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