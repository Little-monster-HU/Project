#pragma once

#include<iostream>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/sendfile.h>
#include<sys/stat.h>
#include<unordered_map>
#include<vector>
#include<fcntl.h>
#include<strings.h>
#include<string>
#include<unistd.h>
#include<sys/wait.h>
#include"Tool.hpp"

//using namespace std;

#define www "./wwwroot"

class Request{
	private:
		string Requestline;
		string Requesthead;
		string Requestkong;
		string Requestbody;
	private:
		string method;
		string uri;
		string vesion;
		unordered_map<string, string> head_kv;

		string path;
		string Queue_str; 
		bool cgi;
		int recource_size;
		string suffix;

	public:
		Request():Requestkong("\n"),path(www),cgi(false)
		{
		}
		string &GetRequestline()
		{
			return Requestline;

		}
		string &GetRequesthead()
		{
			return Requesthead;
		}
		string &GetRequestbody()
		{
			return Requestbody;

		}
		string &Getvesion()
		{
			return vesion;
		}
		string &Getpath()
		{
			return path;
		}
		void RequestlineParse()
		{
			stringstream ss(Requestline);
			ss>>method>>uri>>vesion;
		}
		bool MethodIslegal(){
			// GET || POST
			Function::mytoupper(method);
			cout << "method:"<<method<<endl;
			cout << "uri:" << uri <<endl;
			cout << "vesion:" << vesion << endl;

			if(method != "GRT" && method != "POST")
				return true;	
			return false; 
		}

		void RequestheadParse()
		{
			vector<string> v;
			Function::HeadtoVector(GetRequesthead(),v);
		
			for(auto it = v.begin();it!=v.end();it++){
			string k;
			string v;
			Function::Makekv(*it, k, v);
			head_kv.insert(make_pair(k,v));
	 		}
			
		}

		bool IsneedRecv()
		{
			return (method=="POST")?true:false;
		}
		int GetContentLength()
		{
			auto it = head_kv.find("Content-Length");
			if(it == head_kv.end())
			{				
				return -1;

			}
			return Function::StrtoInt(it->second);
		}

		bool Uriparse()
		{
			if(method == "POST")
			{
				cgi = true;
			}
			else {
				auto pos = uri.find('?');

				if(string ::npos==pos)
				{
					path+=uri;
				}
				else
				{

					cout<<"cgi have ?!" << endl;
					cgi = true ;
					path = uri.substr(0,pos);
					path = uri.substr(pos+1);
					int pos1 = path.rfind('.');
					suffix=path.substr(pos1);
				}
			}
			if(path[path.size()-1]=='/')
				path+="index.html";
		}

		bool Ispathlegal()
		{
			struct stat s;
			if(stat(path.c_str(),&s)==0)
			{
				if(S_ISDIR(s.st_mode))

				{
					path+="/index.html";
				}
				else{
					if(s.st_mode&S_IXUSR||s.st_mode&S_IXGRP||s.st_mode&S_IXOTH)
					{
						cout<< "cgi is  .exe!"<<endl;
					
					
						cgi=true;
			
					}

				}
				recource_size=s.st_size;
				return true;

			}			
			else
				return false;
		}

		string &Getpara()
		{
			if(method=="Get")
				return Queue_str;
			return Requestbody;
		}
		int GetrecourceSize()
		{
			return recource_size;
		}
		string &Getsuffix()
		{
			return suffix;
		}
		bool Iscgi()
		{
			return cgi;
		}
		bool Getcgi()
		{
			return Iscgi();
		}

		~Request()
		{
		}
};
	
class Respond{
	private:
		string Respondline;
		string Respondhead;
		string Respondkong;
		string Respondbody;
		string suffix;
		string path;

		int fd;
		int size;
	public:
		Respond():Respondkong("\n"),fd(-1)
	{

	}
		string &GetRespondline()
		{
			return Respondline;
		}
		string &GetRespondhead()
		{
			return Respondhead;
		}
		string &GetRespondkong()
		{
			return Respondkong;
		}
		string &GetRespondbody()
		{
			return Respondbody;
		}
		void Make_404()
		{
			suffix=".html";
			path="wwwroot/404.html";
			struct stat s;
			stat(path.c_str(),&s);
			size=s.st_size;
		}
		void Remake(int code)
		{
			switch(code)
			{
				case 404:

					
					Make_404();
					break;
				default:
					break;
			}
		}
		void MakeRespondline(int code,Request *rqs)
		{
			Respondline="HTTP/1.1";
			Respondline+=Function::InttoStr(code);
			Respondline+=" ";
			switch(code)
			{
				case 200:
					Respondline+="OK";
					break;
				case 404:
					
					Respondline+="Not Found";
					break;
				default:
					
					Respondline+="Unkown error";
					break;
			}
			Respondline+="\r\n";
		}
		void MakeRespondhead(vector<string>& v)
		{
			auto it=v.begin();
			for(;it!=v.end();it++)
			{
				Respondhead+=*it;

				Respondhead+="\r\n";
			}
		}
		void MakeRespond(Request *rqs,int code,bool cgi)
		{
			cout<<"Makeresponse"<<endl;
			MakeRespondline(code,rqs);
			vector<string> v;
			if(cgi)
			{
				
				cout << "is cgi" << endl;
				suffix=Function::SuffixtoType("");
				v.push_back(suffix);
				string len = "Content-Length: ";
				len+=Function::InttoStr(Respondbody.size());
				v.push_back(len);
			    MakeRespondhead(v);
			}
			else{
				
				if(code==200)
				{
					suffix=rqs->Getsuffix();

					path=rqs->Getpath();
					cout<<"debug: "<<path<<endl;

					size=rqs->GetrecourceSize();
				}
				cout <<"suffix"<<suffix<<endl;
				cout <<"size"<<size<<endl;
				cout<<"path"<<path <<endl;
				fd=open(path.c_str(),O_RDONLY);
				string str=Function::SuffixtoType(suffix);
				string len="Content-Length: ";
				len += Function::InttoStr(size);
				v.push_back(str);
				v.push_back(len);
				MakeRespondhead(v);
				cout<<"Makerespond success!"<<endl;
				cout<<"Respondline: "<<Respondline<<endl;
				cout<<"Respondhead: "<<Respondhead<<endl;
				cout<<"Respondkong: "<<Respondkong<<endl;
				cout<<"Respondbody: "<<Respondbody<<endl;
			}
		}
		int Getfd()
		{
			return fd;
		}
		int Getsize()
		{
			return size;
		}
		~Respond()
		{
			if(fd<0)
				close(fd);
		}
};

class EnPoint
{
	private:
		int sock;
	public:
		EnPoint(int sock_):sock(sock_)
	{

	}
		int Recvline(string &r)
		{
			char ch='a';
			while(ch!='\n')
			{
				int s=recv(sock,&ch,1,0);
				if(s<0)
				{
			
					ch='\n';
				
					r.push_back(ch);
				}
				else{

			
					
					
					if(ch!='\n'&&ch!='\r')
					{
						r.push_back(ch);
					}
					else
					{
			
						if(ch=='\r')
						{
					
							recv(sock,&ch,1,MSG_PEEK);
						if(ch=='\n')
							recv(sock,&ch,1,0);
						else
							ch='\n';
					}
					r.push_back(ch);
				}
			}
		}
		return r.size();
		cout << "recv success!"<<endl;
}

		void RecvRequestline(Request *rq)
{
	Recvline(rq->GetRequestline());
}
		void RecvRequesthead(Request *rq)
		{	
			string &h = rq ->GetRequesthead();
			do{
			string ret = "";
			Recvline(ret);
			if(ret == "\n" ){
				break;
			}
			h += ret;
			}while(1);
		
		}

		void RecvRequestbody(Request *rq)
{
	int len = rq->GetContentLength();
	string &body = rq->GetRequestbody();
	char c;
	while(len--)
	{
		if(recv(sock,&c,1,0)>0)
		{

			body.push_back(c);
		}
	}
}
void SendRespond(Respond *rps,int sock,bool cgi)
{
	cout<<"send1..."<<endl;
	string &respondline=rps->GetRespondline();
	string &respondhead=rps->GetRespondhead();
	string &respondkong=rps->GetRespondkong();
	send(sock,respondline.c_str(),respondline.size(),0);
	send(sock,respondhead.c_str(),respondhead.size(),0);
	send(sock,respondkong.c_str(),respondkong.size(),0);
	if(cgi)
	{

		string body=rps->GetRespondbody();
		send(sock,body.c_str(),body.size(),0);
	}
	else
	{

		int fd=rps->Getfd();
		int size = rps->Getsize();
		sendfile(sock,fd,NULL,size);
	}
}

void ClearRequest(Request *rqs)
{
	if(rqs->GetRequesthead().empty())
	{
		RecvRequesthead(rqs);
	}
	if(rqs->IsneedRecv())
	{
		if((rqs->GetRequestbody().empty()))
			RecvRequestbody(rqs);
	}
}
		
		~EnPoint()
		{
			close(sock);
			cout<<"quit..."<<endl;
		}


};

class Entry{
	private:


	public:

	static int ProcessCgi(EnPoint *ep,Request *rqs,Respond *rps)
	{
		pid_t id = fork();
		int input[2]= {0};
		int output[2]= {0};
		pipe(input);
		pipe(output);
		string path=rqs->Getpath();
		string para=rqs->Getpara();
		int size=rqs->GetrecourceSize();
		string contentlength="content-Length=";
		contentlength+=Function::InttoStr(size);
		string text;

		if(id<0)
		{
			cout << "fork errer1" << endl;
		return 502;
		}
		else if(id ==0)
		{
			close(input[1]);
			close(output[0]);
			putenv((char *)contentlength.c_str());
			dup2(input[0],0);
			dup2(output[1],1);
			execl(path.c_str(),path.c_str(),nullptr);
			cout<<"execl false!"<<endl;
			exit(1);

		}
		else{
			close(input[0]);
			close(output[1]);
			for(auto it=para.begin();it!=para.end();it++)
			{
				char ret=*it;
				write(input[1],&ret,1);
			}
			waitpid(id,NULL,0);
			char c;
			while(read(output[0],&c,1)>0)
			{
				text.push_back(c);
			}
		}
		return 200;
	}

	static void *HandleRequest(int sock)
	{
		cout<<"enter Handler"<<endl;
		int code =200;
		EnPoint *ep=new EnPoint(sock);
		Request *rqs=new Request();
		Respond *rps=new Respond();
		ep->RecvRequestline(rqs);
		rqs->RequestlineParse();
		if(!rqs->MethodIslegal())
		{
			code =404;
			goto end;
		}
		ep->RecvRequesthead(rqs);
		cout<<rqs->GetRequesthead()<<endl;
		rqs->RequestheadParse();
		if(rqs->IsneedRecv())
		{

			ep->RecvRequestbody(rqs);
		}
		rqs->Uriparse();
		if(!rqs->Ispathlegal())
		{
			code=404;
			cout<< "path no  legal!"<<endl;
			goto end;
		}
		cout << "path legal!"<<endl;
		if(rqs->Getcgi())
		{
			cout << "do cgi!"<<endl;
			code=ProcessCgi(ep,rqs,rps);
			if(code == 200)
			{
				rps->MakeRespond(rqs,code,rqs->Getcgi());
				ep->SendRespond(rps,sock,true);
			}
		}
		else{
			cout << "no do cgi!" <<endl;
			rps->MakeRespond(rqs,code,rqs->Getcgi());
			ep->SendRespond(rps,sock,false);
		}
	end:
		if(code!=200)
		{
			ep->ClearRequest(rqs);
			rps->Remake(code);
			rps->MakeRespond(rqs,code,false);
			ep->SendRespond(rps,sock,false);
		}
		delete ep;
		ep=NULL;
		delete rqs;
		rqs=NULL;
		delete rps;
		rps=NULL;

	}
	~Entry()
	{

	}

};
