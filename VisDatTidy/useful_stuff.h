#pragma once

namespace useful {
	int schift = 0;
	bool dbg = false;

	void toClipboard(const std::string &s) {
		OpenClipboard(0);
		EmptyClipboard();
		HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, s.size());
		if (!hg) {
			CloseClipboard();
			return;
		}
		memcpy(GlobalLock(hg), s.c_str(), s.size());
		GlobalUnlock(hg);
		SetClipboardData(CF_TEXT, hg);
		CloseClipboard();
		GlobalFree(hg);
	}

	unsigned long rgb16_to_rgb32(unsigned short a) {
		bool dbg = false;
		/* 1. Extract the red, green and blue values */
		if (dbg)
			std::cout << "Value read: 0x" << std::hex << a << std::endl;
		/* from rrrr rggg gggb bbbb */
		unsigned long r = (a & 0xF800) >> 11;
		unsigned long g = (a & 0x07E0) >> 5;
		unsigned long b = (a & 0x001F);

		/* 2. Convert them to 0-255 range:
		There is more than one way. You can just shift them left:
		to 00000000 rrrrr000 gggggg00 bbbbb000
		r <<= 3;
		g <<= 2;
		b <<= 3;
		But that means your image will be slightly dark and
		off-colour as white 0xFFFF will convert to F8,FC,F8
		So instead you can scale by multiply and divide: */

		r = r * 255 / 31;
		g = g * 255 / 63;
		b = b * 255 / 31;


		/* This ensures 31/31 converts to 255/255 */

		/* 3. Construct your 32-bit format (this is 0RGB): */
		//return (r << 16) | (g << 8) | b;
		DWORD res = ((r << (24 + schift)) | (g << (16 + schift)) | b << (8 + schift) | (0xFF) >> (0 + schift));
		if (dbg) {

			std::cout << "R: 0x" << std::hex << r << " G: 0x" << g << " B: 0x" << b << std::endl;
			std::cout << "Result: 0x" << std::hex << res << std::endl;
			system("pause");
		}
		return res;

		// Or for BGR0:
		//return (r << 24) | (g << 16) | (b << 0);

	}


	unsigned long rgb8_to_rgb32(BYTE a) {
		
		/* 1. Extract the red, green and blue values */
		if (dbg)
			std::cout << "Value read: 0x" << std::hex << a << std::endl;
		/* from rrrr rggg gggb bbbb */
		//unsigned long r = (a >> 5) * 255 / 7;
		//unsigned long g = ((a >> 2) & 0x07) * 255 / 7;
		//unsigned long b = (a & 0x03) * 255 / 3;

		unsigned long r = (a >> 5) * 32;
		unsigned long g = ((a & 28) >> 2) * 32;
		unsigned long b = (a & 0x03) * 64;

		/* 2. Convert them to 0-255 range:
		There is more than one way. You can just shift them left:
		to 00000000 rrrrr000 gggggg00 bbbbb000
		r <<= 3;
		g <<= 2;
		b <<= 3;
		But that means your image will be slightly dark and
		off-colour as white 0xFFFF will convert to F8,FC,F8
		So instead you can scale by multiply and divide: */

		//r = r * 255 / 31;
		//g = g * 255 / 63;
		//b = b * 255 / 31;


		/* This ensures 31/31 converts to 255/255 */

		/* 3. Construct your 32-bit format (this is 0RGB): */
		//return (r << 16) | (g << 8) | b;
		DWORD res = ((r << (24 + schift)) | (g << (16 + schift)) | b << (8 + schift) | (0xFF) >> (0 + schift));
		if (dbg) {

			std::cout << "R: 0x" << std::hex << r << " G: 0x" << g << " B: 0x" << b << std::endl;
			std::cout << "Result: 0x" << std::hex << res << std::endl;
			system("pause");
		}
		return res;

		// Or for BGR0:
		//return (r << 24) | (g << 16) | (b << 0);

	}

	std::string ExePath() {
		char buffer[MAX_PATH];
		GetModuleFileName(NULL, buffer, MAX_PATH);
		std::string::size_type pos = std::string(buffer).find_last_of("\\/");
		return std::string(buffer).substr(0, pos);
	}

}