#include<iostream>
#include<string>
//#include<algorithm>
#include<sstream>

using namespace std;

int main()
{
	string s = "content-length: 1234\na:b\nc:d\nefg:321\n";
	size_t pos = s.find_first_of("\n");
	if(std::string::npos == pos){
		return 1;
	}
	cout << pos << endl;
	string sub = s.substr(0,pos);
	cout << sub <<endl;
	//string s = "hello world";
	//transform(s.begin() , s.end() , s.begin(), ::toupper);
	//cout << s << endl;
	//transform(s.begin(), s.end(), s.begin(),  ::tolower);
	//cout << s << endl;


	//	string s = "GET     /index.html   http/1.1";
	//	std::stringstream ss(s);
	//	string method;
	//	string uri;
	//	string version;
	//	ss >> method >> uri >>version;

	//	cout << "method:" << method << endl;
	//	cout << "uri  : " << uri << endl;
	//	cout << "version:" << version << endl;




}



