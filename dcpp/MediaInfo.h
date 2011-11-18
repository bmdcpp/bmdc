//-----------------------------------------------------------------------------
//(c) 2011 http://flylinkdc.blogspot.com, pavel.pimenov@gmail.com
//-----------------------------------------------------------------------------
#ifndef MediaInfo_H
#define MediaInfo_H

#include <string>

class MediaInfo
{
	public:
		uint16_t m_bitrate;
		uint16_t m_mediaX;
		uint16_t m_mediaY;
		std::string m_video;
		std::string m_audio;
		MediaInfo()
		{
			init();
		}
		MediaInfo(const std::string& p_WH, uint16_t p_bitrate = 0)
		{
			init(p_WH,p_bitrate);
		}
		void init(const std::string& p_WH, uint16_t p_bitrate = 0)
		{
			init(p_bitrate);
			auto l_pos = p_WH.find('x');
			if (l_pos != std::string::npos)
			{
				m_mediaX = atoi(p_WH.c_str());
				m_mediaY = atoi(p_WH.c_str() + l_pos + 1);
			}
		}
		void init(uint16_t p_bitrate = 0)
		{
			m_bitrate = p_bitrate;
			m_mediaX  = 0;
			m_mediaY  = 0;
		}
};
#endif
