/*
 * Copyright (C) Mank 2013-2014 freedcpp at seznam  dot cz
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "SimpleXML.h"
#include "File.h"
#include <string>
#include <map>
using std::string;
using std::map;
using namespace dcpp;

string Escape(string str)
{
	string tmp = str;
	tmp.replace(0,str.find("\\")+1,"");
	
	while(tmp[tmp.length() - 1] == '\\') {
			tmp.erase(tmp.length()-1);
			tmp[tmp.length()-1] = '/';
	}	
	return tmp;	
}

int main(int argc ,char *argv[]) {

	if(argc <= 1) return 1;
	
	map<string,string> emoticons;
	SimpleXML xml;
	xml.fromXML(File( string(argv[1]) + ".xml", File::READ, File::OPEN).read());
	try {

		if(xml.findChild("Emoticons")) 
		{
			xml.stepIn();
			printf("test");
			while(xml.findChild("Emoticon")) {
				string strEmotionText = xml.getChildAttrib("PasteText");
				printf("%s", strEmotionText.c_str());
				string strEmotionBmpPath = xml.getChildAttrib("Bitmap");
				printf("%s", strEmotionBmpPath.c_str());
				emoticons.insert( make_pair(strEmotionText, Escape(strEmotionBmpPath)));
			}
		}
	} catch(...) {
		printf("EmoticonsManager::Create:1 \n");
	}
	//czdc
try{
		xml.resetCurrentChild();
		if(xml.findChild("Emotions")) {
			xml.stepIn();
			printf("test");
			while(xml.findChild("Emotion")) {
			string strEmotionText = xml.getChildAttrib("ReplacedText");
								printf("%s", strEmotionText.c_str());

				string strEmotionBmpPath = xml.getChildAttrib("BitmapPath");
								printf("%s", strEmotionBmpPath.c_str());

				emoticons.insert(make_pair(strEmotionText, Escape(strEmotionBmpPath)));
			}
			xml.stepOut();	
		}	
	} catch(...) {
		printf("EmoticonsManager::Create:2 \n");
	}
		printf("size %d",emoticons.size());
		SimpleXML aXml;
		aXml.addTag("emoticons-map");
		aXml.addChildAttrib("name",Util::getFileName(argv[1]));
		aXml.stepIn();
		for(auto emot = emoticons.begin();emot!=emoticons.end();++emot) {
				aXml.addTag("emoticon");
				aXml.addChildAttrib("file",emot->second);
				aXml.stepIn();
				aXml.addTag("name");
				aXml.addChildAttrib("text",emot->first);
				aXml.stepOut();
		}
		aXml.stepOut();
		
		string fname = string(argv[1]) + ".xml.bmdc.xml";
	try {	
		File f(fname, File::WRITE, File::CREATE | File::TRUNCATE);
		f.write(SimpleXML::utf8Header);
		f.write(aXml.toXML());
		f.close();
	}catch(...){ printf("Not Safe"); return 1;}	
		return 0;

}
