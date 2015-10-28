// Spell Suggestor
// Author : Tanmay Patil

#define _CRT_SECURE_NO_WARNINGS

#include <algorithm>
#include <iostream>
#include <string>
#include <fstream>
#include <unordered_set>
#include <ctime>

using namespace std;


#define MAX_DISTANCE_SUGGESTIONS 1
#define MAX_SUGGESTIONS			 5 

/* MAXIMUM LENGTH OF THE GROUP OF TOKENS READ WITH A WHITE SPACE*/
#define MAX_GROUP_TOKENS_SIZE 50


class input_text
{
private:
	ifstream input_file;
	
	bool tokens_left;
	char str_tokens[MAX_GROUP_TOKENS_SIZE];
public:
	input_text(string fname)
	{
		
		input_file.open(fname);
		if(input_file.fail())
		{
			cout<< "Error opening input file.";
			exit(1);
		}

		tokens_left = false;
	}
	~input_text()
	{
		input_file.close();
	}

	string get_next_word(void)
	{
		string next_word;
		char* token;
		
		
		if (tokens_left)
		{
			token = strtok (NULL,"_,.-?/\";:!");

			if (token == NULL)
				tokens_left = false;
			else{
				strlwr(token);
				string ret_word(token);
				return ret_word;
			}
		}

		if(!tokens_left)
		{
			do{
				input_file>> next_word;

				if(next_word.empty())
					return next_word;


				strcpy(str_tokens,  next_word.c_str());
				token = strtok( str_tokens , "_,.-?/\";:!" );

			}while( token == NULL);

			tokens_left = true;
			strlwr(token);
			string ret_word(token);
			return ret_word;
		}

		return NULL;
	}
};

class output_text
{
private:
	ofstream output_file;


public:
	output_text(string fname)
	{

		output_file.open(fname, ios::out | ios::trunc);
		if(output_file.fail())
		{
			cout<< "Error opening output file.";
			exit(1);
		}
	}
	~output_text()
	{
		output_file.close();
	}

	void write_word(string word)
	{
		
		output_file << word + " ";

		
	}

	void write_word_suggestions( string word, vector<string>  suggestions)
	{
		string out_word = "[" + word + "] {";

		for(int i=0;i<(int)suggestions.size();i++)
			out_word = out_word + suggestions[i] + ((i < (int)suggestions.size()-1 )? ", " : "");

		out_word += "}";

		write_word(out_word);
	}
};

class dictionary
{
private:
	ifstream dict_file;

	unordered_set<string> dict_set;

	//list<string> dict_list;

public:
	dictionary(string fname)
	{
		
		//dict_set = unordered_set<string>(1000);

		dict_set.max_load_factor(10);
		//dict_set.rehash(10000);
		

		dict_file.open(fname);

		if(dict_file.fail())
		{
			cout << "Error: " << strerror(errno);
		}
		string word;
		
		while(dict_file >> word)
		{
			dict_set.insert(word);
			//dict_list.insert(word);
		}

		if(dict_set.size() == 0)
		{
			cout << "Dictionary empty";
		}

	}
	~dictionary()
	{
		dict_file.close();
		dict_set.clear();
	}

	bool is_present(string search_word)
	{
		//std::transform(search_word.begin(), search_word.end(), search_word.begin(), ::tolower);

		unordered_set<string>::iterator dict_set_iterator;

		dict_set_iterator = dict_set.find(search_word);

		if(dict_set_iterator == dict_set.end())
			return false;
		else
			return true;

	}
};

class spell_suggestion_generator
{
private:
	int max_suggestions;

	int max_delete_dist;
	int max_replace_dist;
	int max_insert_dist;

	dictionary *dict;


	// FUNCTION POINTER TO OPERATIONS
	vector<string> (spell_suggestion_generator::*get_suggestions_operations[4]) (string , int, int);

public:
	
	spell_suggestion_generator(dictionary * input_dict)
	{
		max_suggestions = MAX_SUGGESTIONS;
		dict = input_dict;

		max_delete_dist =	MAX_DISTANCE_SUGGESTIONS;
		max_replace_dist =  MAX_DISTANCE_SUGGESTIONS;
		max_insert_dist =	MAX_DISTANCE_SUGGESTIONS;

		get_suggestions_operations[0] = &spell_suggestion_generator::get_suggestions_replace;
		get_suggestions_operations[1] = &spell_suggestion_generator::get_suggestions_delete;
		get_suggestions_operations[2] = &spell_suggestion_generator::get_suggestions_insert;
	}

	// FIND FOR SUGGESTIONS WITH DELETE OPERATIONS
	vector<string> get_suggestions_delete(string mispelled_string , int dist, int req_suggestions)
	{
		vector<string> suggested_strings;

		if((int)mispelled_string.size() > dist)
		{
			string::iterator i;
			int pos = 0;
			for(i = mispelled_string.begin(); i != mispelled_string.end(); i++, pos++ )
			{
				string deleted_string = mispelled_string;
				deleted_string.erase(pos, 1);

				if(dist > 1)
					suggested_strings = get_suggestions_delete(deleted_string, dist-1, req_suggestions);
				else
				{
					if(dict->is_present(deleted_string))
						suggested_strings.push_back(deleted_string);
					
					if((int)suggested_strings.size() >=  req_suggestions)
						return suggested_strings;
				}
				
			}
		}
		
		return suggested_strings;
	}

	// FIND FOR SUGGESTIONS WITH INSERT OPERATIONS
	vector<string> get_suggestions_insert(string mispelled_string , int dist, int req_suggestions)
	{
		vector<string> suggested_strings;
		bool all_inserted_before_end = false, inserted_at_end = false;

		for(int pos = 0; pos <= (int)mispelled_string.size()  ; pos++ )
		{
			for(char ins_ch='a'; ins_ch <='z'; ins_ch ++)
			{
				string inserted_string = mispelled_string;
				
				if ( pos < (int)mispelled_string.size() )
					inserted_string.insert(pos, 1, ins_ch);
				else
					inserted_string.push_back(ins_ch);

				if(dist > 1)
					suggested_strings = get_suggestions_insert(inserted_string, dist-1, req_suggestions);
				else
				{
					if(dict->is_present(inserted_string))
						suggested_strings.push_back(inserted_string);

					if((int)suggested_strings.size() >=  req_suggestions)
						return suggested_strings;
				}
			}

			
		}

		return suggested_strings;
	}

	// FIND FOR SUGGESTIONS WITH REPLACE OPERATIONS
	vector<string> get_suggestions_replace(string mispelled_string , int dist, int req_suggestions)
	{
		vector<string> suggested_strings;

		string::iterator i;
		int pos = 0;
		for(i = mispelled_string.begin(); i != mispelled_string.end(); i++, pos++)
		{
			for(char ins_ch='a'; ins_ch <='z'; ins_ch ++)
			{
				string replaced_string = mispelled_string;
				replaced_string.replace(pos, 1, 1, ins_ch);

				if(dist > 1)
					suggested_strings = get_suggestions_replace(replaced_string, dist-1, req_suggestions);
				else
				{
					if(dict->is_present(replaced_string))
						suggested_strings.push_back(replaced_string);

					if((int)suggested_strings.size() >=  req_suggestions)
						return suggested_strings;
				}
			}
		}

		return suggested_strings;
	}

	

	// FIND SUGGESTIONS MAIN ALGORITHM
	vector<string> get_suggestions(string mispelled_string)
	{
		vector<string> suggested_strings, temp;
		int req_suggestions = max_suggestions;

		for (int d = 1; d <= MAX_DISTANCE_SUGGESTIONS; d++ )
		{
			for (int op = 0; op < 3; op ++)
			{
				temp = (this->*get_suggestions_operations[op])(mispelled_string, d, req_suggestions);

				req_suggestions -= (int)temp.size();

				suggested_strings.insert(suggested_strings.end(), temp.begin(), temp.end());

				if((int)suggested_strings.size() >= max_suggestions )
					return suggested_strings;
			}
		}

		return suggested_strings;
	}
};

class spell_checker
{
	dictionary *dict;
public:

	spell_checker(dictionary * input_dict)
	{
		dict = input_dict;
	}

	bool is_good_word(string word)
	{
		char ch = word.at(0);
		
		// Ignore words not starting with alphabets 
		if( !( (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ) )
		{
			return true;
		}
		
		if ( dict->is_present(word) )
		{
			return true;
		}

		return false;
	}
};

void main()
{
	clock_t begin = clock();

	cout<< "\nReading input file...";
	input_text in_text("input.txt") ;
	
	cout << "\nReading output file...";
	output_text out_text("output.txt");
	
	cout<< "\nReading dictionary...";
	dictionary dict("dictionary.txt");

	cout << "\nElapsed time = " << double(clock() - begin) / CLOCKS_PER_SEC;

	spell_checker spell_checking(&dict);
	spell_suggestion_generator spell_correction(&dict);

	cout<< "\nFinding mis-spelled words.."; 
	while(1)
	{
		string word = in_text.get_next_word();

		if(word.empty())
			break;

		if(spell_checking.is_good_word(word))
		{
			out_text.write_word(word);
		}
		else
		{
			vector<string> suggestions = spell_correction.get_suggestions(word);
			out_text.write_word_suggestions(word, suggestions);
		}
	}
	cout << "\nElapsed time = " << double(clock() - begin) / CLOCKS_PER_SEC;

	cout<< "\nEnter to exit.";
	getchar();


	cout << "\nDestroying memory assigned...";
	
}
