using System;
using System.Windows.Forms;
using MySql.Data.MySqlClient;

namespace NewPatientCare
{
	
	public class DBConnect
	{

		private MySqlConnection connection;
		private String server;
		private String database;
		private String uid;
		private String password;

		public DBConnect ()
		{
			Initialize();
		}

		private void Initialize(){
			server = "s246.loopia.se";
			database = "markusaxelsson_se_db_1";
			uid = "markus";
			password = "axelsson78";
			String connectionString;
			connectionString = "SERVER=" + server + ";" + "DATABASE=" + database
			+ ";" + "UID=" + uid + ";" + "PASSWORD=" + password + ";";

			connection = new MySqlConnection(connectionString);
		}

		private bool openConnection(){
			try{
				connection.Open();
				return true;
			}
			catch ( MySqlException ex ){
				//When handling errors, you can your application's response based 
				//on the error number.
				//The two most common error numbers when connecting are as follows:
				//0: Cannot connect to server.
				//1045: Invalid user name and/or password.

				switch (ex.Number) {
				case 0:
					MessageBox.Show("Cannot connect to server.  Contact administrator");
					break;

				case 1045:
					MessageBox.Show("Invalid username/password, please try again");
					break;
				}
				return false;
			}
		}

		private bool closeConnection(){
			try
			{
				connection.Close();
				return true;
			}
			catch (MySqlException ex)
			{
				MessageBox.Show(ex.Message);
				return false;
			}
		}

		public List< string >[] Select()
		{
			string query = "SELECT * FROM user";

			//Create a list to store the result
			List< string >[] list = new List< string >[3];
			list[0] = new List< string >();
			list[1] = new List< string >();
			list[2] = new List< string >();

			//Open connection
			if (this.OpenConnection() == true)
			{
				//Create Command
				MySqlCommand cmd = new MySqlCommand(query, connection);
				//Create a data reader and Execute the command
				MySqlDataReader dataReader = cmd.ExecuteReader();

				//Read the data and store them in the list
				while (dataReader.Read())
				{
					list[0].Add(dataReader["id"] + "");
					list[1].Add(dataReader["name"] + "");
					list[2].Add(dataReader["age"] + "");
				}

				//close Data Reader
				dataReader.Close();

				//close Connection
				this.CloseConnection();

				//return list to be displayed
				return list;
			}
			else
			{
				return list;
			}
		}
	}
}

