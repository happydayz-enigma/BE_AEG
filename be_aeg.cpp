/*
	BattlEye Auto-Exception Generator (BE_AEG)
	Created by eraser1

	This work is protected by Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0). By using, downloading, or copying any of the work contained, you agree to the license included.
	http://creativecommons.org/licenses/by-nc-sa/4.0/


The following overview is a human-readable summary of (and not a substitute for) [the full license](http://creativecommons.org/licenses/by-nc-sa/4.0/legalcode).

You are free to:

Share � copy and redistribute the material in any medium or format.
Adapt � remix, transform, and build upon the material.


Under the following terms:

Attribution � You must give appropriate credit, provide a link to the license, and indicate if changes were made. You may do so in any reasonable manner, but not in any way that suggests the licensor endorses you or your use.
NonCommercial � You may not use the material for commercial purposes.
ShareAlike � If you remix, transform, or build upon the material, you must distribute your contributions under the **same license** as the original.
No additional restrictions � You may not apply legal terms or technological measures that legally restrict others from doing anything the license permits.


Notices:

You do not have to comply with the license for elements of the material in the public domain or where your use is permitted by an applicable exception or limitation
No warranties are given. The license may not give you all of the permissions necessary for your intended use. For example, other rights such as publicity, privacy, or moral rights may limit how you use the material.
*/


#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <regex>
#include <map>
#include <ctime>


#define DATE_TIME time_t now = time(0);string dt(ctime(&now));dt.pop_back()


using namespace std;


// Value-Definitions of the different String values
static enum StringValue
{
	ev_undefined,
	ev_noScriptLogBackup,
	ev_noLogs,
	ev_enableDebugConsole,
	//ev_noScripts,
	ev_runOnce
};

// Map to associate the strings with the enum values
static map<string, StringValue> s_mapStringValues;



/* HELPER FUNCTIONS */
string fileToString(string fileName)
{
	ifstream file(fileName);
	string fileStr;
	/*
	file.seekg(0, ios::end);
	fileStr.reserve(file.tellg());
	file.seekg(0, ios::beg);
	*/
	fileStr.assign( ( istreambuf_iterator<char>(file) ), istreambuf_iterator<char>() );

	return fileStr;
};


bool in_vector(vector<string> vec, string find)
{
	if (vec.size() == 0) return false;
	for (unsigned int i=0; i < vec.size(); ++i)
	{
		if (vec[i] == find) return true;
	}

	return false;
};


string vector_get(vector<string>& vec, unsigned int index)
{
	if (vec.size() < (index + 1))
	{
		vec.resize(index + 1);
	}

	return vec[index];
}


vector<string> vector_get(vector< vector<string> >& vec, unsigned int index)
{
	if (vec.size() < (index + 1))
	{
		vec.resize(index + 1);
	}

	return vec[index];
}



int main(int argc, char* argv[])
{
	// Variable initialization

	// Define regular expressions
	regex rgx_script_restr ( "\\(\\d+?\\.\\d+?\\.\\d+?\\.\\d+?:\\d+?\\)\\s\\w+\\s-\\s#(\\d)\\s\"((?:(?!\\n\\d\\d\\.\\d\\d\\.\\d\\d\\d\\d\\s\\d\\d:\\d\\d:\\d\\d:\\s)(?:\\s|\\S))*)" );	// The single line that took over 12 hours of work.
	regex rgx_match_newlines("\\n");

	vector< vector<string> > previous_exceptions;		// This is so we don't add duplicate (script) exceptions


	// Launch options
	bool backup_script_logs = true;
	bool use_debug_log = true;
	bool use_debug_console = false;
	bool run_once = false;
	//bool generate_script_exceptions = true;


	// Initialize the string map
	s_mapStringValues["noScriptLogBackup"] = ev_noScriptLogBackup;
	s_mapStringValues["noLogs"] = ev_noLogs;
	s_mapStringValues["enableDebugConsole"] = ev_enableDebugConsole;
	s_mapStringValues["runOnce"] = ev_runOnce;
	//s_mapStringValues["noScripts"] = ev_noScripts;

	string launch_params = "";
	for (int i=1; i < argc; ++i)
	{
		string param(argv[i]);
		launch_params += param;
		switch (s_mapStringValues[param])
		{
			case ev_noScriptLogBackup:
			{
				backup_script_logs = false;
				cout << "Script logs disabled!\n";
				break;
			}

			case ev_noLogs:
			{
				use_debug_log = false;
				cout << "Debug logs disabled!\n";
				break;
			}

			case ev_enableDebugConsole:
			{
				use_debug_console = true;
				cout << "Debug console enabled!\n";
				break;
			}

			case ev_runOnce:
			{
				run_once = true;
				cout << "BE Auto Exception Generator (BE_AEG) will only run once!\n";
				break;
			}
			/*
			case ev_noScripts:
			{
				generate_script_exceptions = false;
				cout << "Script exceptions will not be generated!\n";
				break;
			}
			*/
			default:
			{
				cout << "Unknown command line parameter: " << argv[i] << endl;
				launch_params += "?";
			}
		}
		launch_params += " ";
	}



	ofstream debug_file;
	if (use_debug_log)
	{
		debug_file.open("BE_AutoExceptionGenerator.log", ios::app);

		DATE_TIME;
		debug_file << "\n\nLAUNCH (" << dt << ") params: " << launch_params << endl;
	}


	scripts_exceptions:
		//if (generate_script_exceptions)
		if (true)
		{
			// Copy file to string
			string fileStr = fileToString("scripts.log");


			if (fileStr == "")
			{
				// scripts.log is empty, so restart the check, or just go to the end if you want to run once.
				if (run_once)
				{
					goto the_end;
				}
				else
				{
					goto scripts_exceptions;
				}
			}

			if (backup_script_logs)
			{
				ofstream scripts_log_backup("BE_AEG_scripts.log", ios::app);
				scripts_log_backup << fileStr;
			}


			smatch match;
			unsigned int ctr = 0;
			vector<string> new_exceptions;						// Initialize the list of exceptions to be added now

			while (regex_search(fileStr,match,rgx_script_restr))
			{
				// Get the restriction # and code that triggered it.
				string restriction = match[1];
				string code = match[2];
				unsigned int restriction_num = atoi(restriction.c_str());

				if (code[code.length() - 1] == '\n')
				{
					code.pop_back();
				}

				// Replace newlines
				string exception = " !=\"" + regex_replace(code,rgx_match_newlines, "\\n");

				vector<string> prev_exceptions_restriction_num = vector_get(previous_exceptions, restriction_num);

				if (!in_vector(prev_exceptions_restriction_num, exception))
				{
					string new_exceptions_line = vector_get(new_exceptions ,restriction_num) + exception;

					new_exceptions[restriction_num] = new_exceptions_line;

					prev_exceptions_restriction_num.push_back(exception);						// Add the exception to the list of exceptions for this restriction number.
					previous_exceptions[restriction_num] = prev_exceptions_restriction_num;		// Update the main exception list.


					if (debug_file.is_open())
					{
						DATE_TIME;
						debug_file << "(" << dt << ") Script Restriction #" << restriction << " produced exception: " << exception << endl;
					}
				}
				else if (debug_file.is_open())
				{
					DATE_TIME;
					debug_file << "(" << dt << ") Script Restriction #" << restriction << " produced DUPLICATED exception: " << exception << endl;
				}


				if (use_debug_console)
				{
					cout << "\n------------------------------------------------------\n" << "Match #" << ctr << endl;
					cout << "Restriction #" << restriction << ":\n" << code << "Exception:\n" << exception << "\n\n";
				}

				++ctr;
				fileStr = match.suffix();
			}

			if (use_debug_console)
			{
				cout << "No More Matches Found!" << endl;
			}


			write_to_scriptsTXT:

			ifstream file("scripts.txt");
			if (!file)
			{
				cout << "Could not access file: \"scripts.txt\"\n";
				// scripts.txt can't be accessed, so we keep trying
				goto write_to_scriptsTXT;
			}

			int current_restriction = -1;
			int new_exceptions_size = new_exceptions.size();
			string scripts_fileStr = "";
			string line;
			while (getline(file, line))
			{
				scripts_fileStr += line;
				if (regex_search(line, regex("^\\d")))
				{
					++current_restriction;

					if (new_exceptions_size >= (current_restriction + 1))
					{
						scripts_fileStr += new_exceptions[current_restriction];
					}
				};
				scripts_fileStr += "\n";
			}
			file.close();

			// Rewrite scripts.txt with the updated exceptions
			ofstream ofile("scripts.txt", ios::trunc);
			ofile << scripts_fileStr;
			ofile.close();

			// Empty the scripts log since we've now generated the exceptions
			ofstream scripts_log("scripts.log", ios::trunc);
			scripts_log.close();
		}
	// End of script exceptions

	if (!run_once)
	{
		goto scripts_exceptions;
	}


	the_end:

	string thisistokeeptheconsoleopen;
	cout << "\n\nBattlEye Auto-Exception Generator is finished running. Type and enter something to exit the program: ";
	cin >> thisistokeeptheconsoleopen;
	return 0;
}



/*
void writeToFile( char const* filename, unsigned int lineNo, string toWrite)
{
    fstream file(filename);
    if (!file)
	{
		cout << "Could not access file:" << filename << "\n";
		return;
	};

    int currentLine = -1;

	string line;
	int prev = file.tellg();
    while (getline(file, line))
    {
		file << line;
		streamoff input_pos = file.tellg();
        // We don't actually care about the lines we're reading,
        // so just discard them.
        //file.ignore( numeric_limits<streamsize>::max(), '\n') ;
		int line_length = line.length();
		if (regex_search(line, regex("^\\d")))
		{
			++currentLine;

			if (currentLine == lineNo)
			{
				 // Position the put pointer
				file.seekp(prev + line_length);

				file << " " << toWrite;
				file.flush();

				file.seekg(input_pos);
			}
		};
		file << endl;
		prev = (int)input_pos;
    }

	//Yes, I kinda sorta have to do this part
	file.close();

	ofstream ofile(filename,ios::app);
	ofile.seekp(0, ios_base::end);
	ofile << "\n";
	ofile.close();
    return;
}
*/