#include <iostream>

#include <string>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Mouse.hpp>
#include <windows.h>
#include "useful_stuff.h"
#include "SimpleIni.h"
#include <sstream>
#include <vector>




class VisDat {
	public:
		/* Other Settings */
		bool FullPageReading = true; //Way faster but cant correct for reading errors
		bool UseVexRender = false; //Setting this to true is waaaay slower than just drawing a bitmap which I didn't know at the time

		/* Debugging */
		bool verboseLogs = true; // as opposed to no logs at all lol

		/*   Process stuff   */
		std::string processWindowTitle = "";
		DWORD StartAddress = 0;
		DWORD  processID = 0;
		HANDLE  processHandle = 0;
							
		DWORD NumberOfBytesToRead = 0;

		/* Control Settings */
		DWORD ScrollSpeed = 20;
		DWORD ScrollSpeedSlow = 1;

		/*   SFML Settings   */
		DWORD sfWinX;
		DWORD sfWinY;
		DWORD sfWinXScaled = 0;
		DWORD sfWinYScaled = 0;
		DWORD sfBitDepth;
		DWORD sfScale;

			/*-> Crap for drawing */
				/*-> Picture */
				sf::Image sfIMG;
				sf::Texture sfTex;
				sf::Sprite sfSpr;

				/*-> VertexArray*/
				sf::VertexArray vex;
				
				
				/*-> Text */
				sf::Font sfFont;
				std::string PathToFontFile = "G:/Script/C++/VisDatReloaded/Debug/arial.ttf";
				sf::Text sfText;
				DWORD sfTextSize;

				/*-> MouseBox */
				sf::RectangleShape MBox;
				
				

		/* Paint Mode */
		int pthickness = 3;
		DWORD col32 = 0xFFFFFFFF;
		WORD  col16 = 0xFFFF;
		bool clickflip = true;
		bool firstclick = true;
		int drawx1, drawy1, drawx2, drawy2;

		/* Mouse vars */
		int MouseX = 0, MouseY = 0;

		/*  Constructor  */
		VisDat(std::string GameWindowTitle,   
			DWORD Scale = 2, 
			DWORD WindowX = 256, DWORD WindowY = 256,
			DWORD BitDepth = 32, 
			DWORD FontSize = 24, 
			DWORD StartAdd = 0x400000,
			bool  VexRender = false,
			bool  FullPage  = false
		) {


			/* Neat Console Title */
			system("title VisDatTidy Console");


			/* Init everything */
			if (GameWindowTitle != "") {
				/* Copy stuff for later use */
				processWindowTitle = GameWindowTitle;
				sfWinX = WindowX;
				sfWinY = WindowY;
				sfWinXScaled = WindowX * Scale;
				sfWinYScaled = WindowY * Scale;
				sfScale = Scale;
				sfBitDepth = BitDepth;
				NumberOfBytesToRead = BitDepth / 8;
				sfTextSize = FontSize;
				StartAddress = StartAdd;
				FullPageReading = FullPage;
				UseVexRender = VexRender;

					//Logs
				if (verboseLogs) { 
					std::cout << "Window title: " << processWindowTitle << std::endl;
					std::cout << "WindowX: " << WindowX << std::endl;
					std::cout << "WindowY: " << WindowY << std::endl;
					std::cout << "Scale: " << Scale << std::endl;
					std::cout << "BitDepth: " << BitDepth << std::endl;
				}
				

				/* Init Hacky stuff */
				if (GetProcessData()) {
					/* Init rendering related stuff */
					sfIMG.create(sfWinXScaled, sfWinYScaled);
					
					/* init vertex array */
					vex.setPrimitiveType(sf::Points);
					vex.resize((sfWinX * sfWinY) * sfScale);

					/* Init Text stuff */
					if (!sfFont.loadFromFile(PathToFontFile))
						SpewErrorAndDie("Could not load font file :(");
					sfText.setFont(sfFont);
					sfText.setCharacterSize(sfTextSize);
					sfText.setFillColor(sf::Color::Green);
						if (verboseLogs) {
							std::cout << "Font OK" << std::endl;
							std::cout << "Start address: 0x" << std::hex << StartAddress << std::dec << std::endl;
							}
				}
				else {
					SpewErrorAndDie("Process not found. Exiting..");
				}
			}
			else {
				SpewErrorAndDie("Game Window Title not valid! Exiting..");
			}

		}

		/* Reading from memory and writing to image */
		void ReadDataAndDrawOnIMG() {
			

			if (!FullPageReading) {
				DWORD ReadAddressCounter = 0;
				for (unsigned int y = 0; y < sfWinYScaled; y += sfScale) {
					for (unsigned int x = 0; x < sfWinXScaled; x += sfScale) {

						/* Check BitDepth to determine reading and drawing style*/
						if (sfBitDepth == 32) {
							BYTE tmp[3] = { 0,0,0 };
							/* Draw only if reading was succ */
							if (ReadProcessMemory(processHandle, (LPVOID)(StartAddress + ReadAddressCounter), &tmp, sizeof(BYTE) * 3, 0)) {
								/* Draw with scaling */
								for (unsigned int yd = 0; yd < sfScale; yd++) {
									for (unsigned int xd = 0; xd < sfScale; xd++) {
										/* Check for BitDepth 32 or 16 */
											
										if (UseVexRender) {
											vex.append(sf::Vertex(sf::Vector2f(x + xd, y + yd), sf::Color(tmp[0], tmp[1], tmp[2], 255)));
										}
										else {
											sfIMG.setPixel(x + xd, y + yd, sf::Color(tmp[0], tmp[1], tmp[2], 255));
										}
										
									}
								}
							}
							else {
								/* Draw read error pattern */
								if (UseVexRender) {
									vex.append(sf::Vertex(sf::Vector2f(x, y), sf::Color::Red));;
								}
								else {
									/* Draw read error pattern */
									for (unsigned int yd = 0; yd < sfScale; yd++) {
										for (unsigned int xd = 0; xd < sfScale; xd++) {

											/* Check for BitDepth 32 or 16 */
											//vex.append(sf::Vertex(sf::Vector2f(x + xd, y + yd), sf::Color::Black));
											sfIMG.setPixel(x + xd, y + yd, sf::Color::Black);
										}
									}
									sfIMG.setPixel(x , y, sf::Color::Red);
								}


								
							}
							ReadAddressCounter += 4;
						}

						/* What if the BitDepth = 16? */
						else if (sfBitDepth == 16) {
							WORD tmp = 0;

							/* Draw only if reading was succ */
							if (ReadProcessMemory(processHandle, (LPVOID)(StartAddress + ReadAddressCounter), &tmp, sizeof(WORD), 0)) {

								/* Draw with scaling */
								for (unsigned int yd = 0; yd < sfScale; yd++) {
									for (unsigned int xd = 0; xd < sfScale; xd++) {

										/* Check for BitDepth 32 or 16 */
										//
										//
										if (UseVexRender) {
											vex.append(sf::Vertex(sf::Vector2f(x + xd, y + yd), sf::Color(useful::rgb16_to_rgb32(tmp))));
										}
										else {
											sfIMG.setPixel(x + xd, y + yd, sf::Color(useful::rgb16_to_rgb32(tmp)));
										}

									}
								}
							}
							else {

								if (UseVexRender) {
									vex.append(sf::Vertex(sf::Vector2f(x, y), sf::Color::Red));;
								}
								else {
									/* Draw read error pattern */
									for (unsigned int yd = 0; yd < sfScale; yd++) {
										for (unsigned int xd = 0; xd < sfScale; xd++) {

											/* Check for BitDepth 32 or 16 */
											//vex.append(sf::Vertex(sf::Vector2f(x + xd, y + yd), sf::Color::Black));
											sfIMG.setPixel(x + xd, y + yd, sf::Color::Black);
										}
									}
									sfIMG.setPixel(x, y, sf::Color::Red);
								}

								
							}
							ReadAddressCounter += 2;
						}
						else {
							SpewErrorAndDie("BitDepth invalid!");
						}
					}
				}
			}
			else { /* Full Page Reading */
				if (sfBitDepth == 32) {
					const unsigned int DataSize = (sfWinXScaled / sfScale)*(sfWinYScaled / sfScale) * 4;
					BYTE *Data = new BYTE[DataSize];
					ReadProcessMemory(processHandle, (LPVOID)StartAddress, Data, DataSize, 0);
					/* Draw with scaling */
					unsigned int vectorindex = 0;
					for (unsigned int y = 0; y < sfWinYScaled; y += sfScale) {
						for (unsigned int x = 0; x < sfWinXScaled; x += sfScale) {
							for (unsigned int yd = 0; yd < sfScale; yd++) {
								for (unsigned int xd = 0; xd < sfScale; xd++) {
									/* Check for BitDepth 32 or 16 */
									if (UseVexRender) {
										vex.append(sf::Vertex(sf::Vector2f(x + xd, y + yd), sf::Color(Data[vectorindex], Data[vectorindex + 1], Data[vectorindex + 2], 255)));
									}
									else {
										sfIMG.setPixel(x + xd, y + yd, sf::Color(Data[vectorindex], Data[vectorindex + 1], Data[vectorindex + 2], 255));
									}
								}
							}
							vectorindex += 4;
						}
					}
					delete[] Data;
				}
				else if (sfBitDepth == 16) {
					const unsigned int DataSize = (sfWinXScaled / sfScale)*(sfWinYScaled / sfScale)*2;
					WORD *Data = new WORD[DataSize];
					ReadProcessMemory(processHandle, (LPVOID)StartAddress, Data, DataSize, 0);
						/* Draw with scaling */
						unsigned int vectorindex = 0;
						for (unsigned int y = 0; y < sfWinYScaled; y += sfScale) {
							for (unsigned int x = 0; x < sfWinXScaled; x += sfScale) {
								for (unsigned int yd = 0; yd < sfScale; yd++) {
									for (unsigned int xd = 0; xd < sfScale; xd++) {
										/* Check for BitDepth 32 or 16 */

										if (UseVexRender) {
											vex.append(sf::Vertex(sf::Vector2f(x + xd, y + yd), sf::Color(useful::rgb16_to_rgb32(Data[vectorindex]))));
										}
										else {
											sfIMG.setPixel(x + xd, y + yd, sf::Color(useful::rgb16_to_rgb32(Data[vectorindex])));
										}
									}
								}
								vectorindex += 1;
							}
						}
					
					delete[] Data;
				}
			}
			ClearIMGVoid();
			PrepareDrawing();
		}
	
		void ClearIMGVoid() {	
			/* Clear the void and draw a line*/
			for (unsigned int y = 0; y < (sfWinY * sfScale); y += sfScale) {
				for (unsigned int x = sfWinXScaled; x < (sfWinX * sfScale); x += sfScale) {

					for (unsigned int xt = 0; xt < sfScale;xt++) {
						for (unsigned int yt = 0; yt < sfScale;yt++) {
							if (UseVexRender) {
								vex.append(sf::Vertex(sf::Vector2f(x + xt, y + yt), sf::Color::Black));
							}
							else {
								sfIMG.setPixel(x + xt, y + yt, sf::Color::Black);
							}

						}
					}


					if (x == sfWinXScaled) {
						for (unsigned int yt = 0; yt < sfScale;yt++) {
							
							if (UseVexRender) {
								vex.append(sf::Vertex(sf::Vector2f(x, y + yt), sf::Color::Magenta));
							}
							else {
								sfIMG.setPixel(x, y + yt, sf::Color::Magenta);
							}
							
						}
						
					}

				}
			}
		}

		void MouseBox() {
			MBox.setSize(sf::Vector2f((pthickness)*sfScale,(pthickness)*sfScale));
			MBox.setPosition(sf::Vector2f((MouseX*sfScale) - (pthickness / 2)*sfScale,(MouseY*sfScale)-(pthickness/2)*sfScale));
			MBox.setOutlineThickness(1);
			MBox.setOutlineColor(sf::Color::Red);
			MBox.setFillColor(sf::Color::Transparent);
		}

		/* Controls */
		void cScroll(int Speed, bool slow) {
			if (slow) {
				StartAddress -= Speed * (sfWinXScaled / sfScale) * NumberOfBytesToRead * ScrollSpeedSlow;
			}else{
				StartAddress -= Speed * (sfWinXScaled / sfScale) * NumberOfBytesToRead * ScrollSpeed;
			}
		}

		void cSetWidth(int Speed) {
			if (Speed < 0 && sfWinXScaled + (Speed*sfScale) > 0) {
				sfWinXScaled += Speed * sfScale;
			}

			if (Speed > 0 && sfWinXScaled + (Speed*sfScale) <= sfWinX*sfScale) {
				sfWinXScaled += Speed * sfScale;
			}
		}

		void cShiftByte(int Speed) {
			StartAddress -= Speed;
		}

		void cPaintDraw() {
			if (firstclick) {
				drawx1 = MouseX;
				drawx2 = MouseX;
				drawy1 = MouseY;
				drawy2 = MouseY;
				firstclick = false;
			}


			if (clickflip) {
				drawx1 = MouseX;
				drawy1 = MouseY;
				DDA(drawx1, drawy1, drawx2, drawy2);
			}
			else {
				drawx2 = MouseX;
				drawy2 = MouseY;
				DDA(drawx2, drawy2, drawx1, drawy1);
			}
			
			clickflip = !clickflip;
		}

		void cPaintDropper() {
			if (sfBitDepth == 32) {
				ReadProcessMemory(processHandle, (LPVOID)(StartAddress + (MouseX + MouseY * (sfWinXScaled / sfScale))*4), &col32, 4, 0);
			}
			else if (sfBitDepth == 16) {
				ReadProcessMemory(processHandle, (LPVOID)(StartAddress + (MouseX + MouseY * (sfWinXScaled / sfScale))*2), &col16, 2, 0);
			}
		}

		void cSaveAddressToClipboard() {
			int step = 4;
			if (sfBitDepth == 16)
				step = 2;

			char last_char;

			std::stringstream clip, temp;
			temp << std::hex << StartAddress + (MouseX + (MouseY*(sfWinXScaled/sfScale))) * step;
			clip << temp.str();
			temp.seekg(-1, std::ios::end);
			temp >> last_char;
			clip << last_char;
			useful::toClipboard(clip.str());
			std::cout << "Address: 0x" << clip.str() << " saved to clipboard!" << std::endl;
		}

		void cSaveAddressToConfig(CSimpleIniA * cfg) {
			int step = 4;
			if (sfBitDepth == 16)
				step = 2;

			std::stringstream clip;
			clip << std::hex << StartAddress;
			cfg->SetValue("settings", "start",std::string(clip.str()).c_str());
			cfg->SaveFile(std::string(useful::ExePath() + std::string("/VisDat_Config.ini")).c_str());
			std::cout << "Current View Address: 0x" << clip.str() << " saved to config!" << std::endl;
		}

		/* Text output */
		void UpdateText(std::string txt) {
			sfText.setString(txt);
		}

		std::string MouseHover() {
			int step = 4;
			if (sfBitDepth == 16)
				step = 2;
			std::stringstream hover;
			hover << std::hex << StartAddress + (MouseX + (MouseY*(sfWinXScaled / sfScale))) * step;
			DWORD read = 0;
			if(ReadProcessMemory(processHandle, (LPVOID)(StartAddress + (MouseX + (MouseY*(sfWinXScaled / sfScale))) * step), &read, step, 0))
				hover << std::endl << "Value: 0x" << std::hex << read << std::endl;
			else
				hover << std::endl << "Value: NO ACCESS" << std::endl;
			return hover.str();
		}

	private:
		

		bool GetProcessData() {
			HWND hWnd = FindWindow(0,processWindowTitle.c_str());
			GetWindowThreadProcessId(hWnd, &processID);
			processHandle = OpenProcess(PROCESS_ALL_ACCESS, false, processID);
			if (processHandle == 0) {
				return false;
				if (verboseLogs) {
					std::cout << "Process Data Error" << std::endl;
				}
			}
			if (verboseLogs) {
				std::cout << "--- Process Data OK ---" << std::endl;
				std::cout << " -> processID: " << processID << std::endl;
				std::cout << " -> processHandle: " << processHandle << std::endl;
			}
			return true;
		}

		/* Error msg and quit */
		void SpewErrorAndDie(std::string Msg) {
			std::cout << Msg << std::endl << std::endl;
			system("pause");
			exit(0);
		}

		/* preparation before calling window.draw() */
		void PrepareDrawing() {
			sfTex.loadFromImage(sfIMG);
			sfSpr.setTexture(sfTex);
		}

		void dbgp(std::string txt, DWORD num) {
			std::cout << txt << num << std::endl << std::endl;
			system("pause");
		}

		void DDA(int X0, int Y0, int X1, int Y1)
		{
			// calculate dx & dy 
			int dx = X1 - X0;
			int dy = Y1 - Y0;

			// calculate steps required for generating pixels 
			int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

			// calculate increment in x & y for each steps 
			float Xinc = dx / (float)steps;
			float Yinc = dy / (float)steps;

			// Put pixel for each step 
			float X = X0;
			float Y = Y0;
			for (int i = 0; i <= steps; i++)
			{
				bool adder = pthickness % 2 == 1;

				if (pthickness > 1) {
					for (int ye = -pthickness / 2; ye < (pthickness / 2) + adder;ye++) {
						for (int xe = -pthickness / 2; xe < (pthickness / 2) + adder;xe++) {
							if (sfBitDepth == 32) {
								VirtualProtectEx(processHandle, (LPVOID)(LPVOID)(StartAddress + (((DWORD)X + xe) + ((DWORD)Y + ye) * (sfWinXScaled / sfScale)) * 4), 4, PAGE_EXECUTE_READWRITE, NULL);
	
								WriteProcessMemory(processHandle, (LPVOID)(StartAddress + (((DWORD)X + xe) + ((DWORD)Y + ye) * (sfWinXScaled / sfScale)) * 4), &col32, 4, 0);
								
							}
							else if (sfBitDepth == 16) {
								WriteProcessMemory(processHandle, (LPVOID)(StartAddress + (((DWORD)X + xe) + ((DWORD)Y + ye) * (sfWinXScaled / sfScale)) * 2), &col16, 2, 0);
							}
						}
					} // put pixel at (X,Y) 
				}
				else {
					if (sfBitDepth == 32) {
						WriteProcessMemory(processHandle, (LPVOID)(StartAddress + (((DWORD)X) + ((DWORD)Y) * (sfWinXScaled / sfScale)) * 4), &col32, 4, 0);
					}
					else if (sfBitDepth == 16) {
						WriteProcessMemory(processHandle, (LPVOID)(StartAddress + (((DWORD)X) + ((DWORD)Y) * (sfWinXScaled / sfScale)) * 2), &col16, 2, 0);
					}
				}
				X += Xinc;           // increment in x at each step 
				Y += Yinc;           // increment in y at each step 
				       // for visualization of line- 
									 // generation step by step 
			}
		}
};


/* Command handling is entirely broken because i have no idea what i'm doing */
class CMD {
	public:
		VisDat * vv;
		bool ErrorTrap = false;

		CMD(VisDat *v) {
			vv = v;
		}
		
		std::string cmdlist = "\nAllowed commands: \n\thelp\n\tgoto\n\texit\n";


		void HandleCommand() {
				if (!ErrorTrap) {
					ErrorTrap = true;
					std::string cmd, arg;
					std::cout << "Command: ";
					std::cin >> cmd;

					if (cmd != "exit") {
						if (cmd != "help") {
							std::cout << "Argument: ";
							std::cin >> arg;
						}
						else {
							cmd_help();
						}


						if (cmd == "goto") {
							cmd_goto(arg);
						}
						else {
							std::cout << "Command not valid. 'help' for more info." << std::endl;
							HandleCommand();
						}
					}
					else {
						std::cout << "Normal operation returned" << std::endl;
						ErrorTrap = false;
						Sleep(50);
					}
				}
			}
		
			

	private:
		void cmd_help() {
			std::cout << cmdlist << std::endl;
			HandleCommand();
		}


		void cmd_goto(std::string add) {
			std::stringstream ss;
			unsigned int start;
			ss << std::hex << add;
			ss >> start;
			vv->StartAddress = start;
			std::cout << "Going to: 0x" << std::hex << start << std::endl;
		}
};


int main() {
	

	/* Load settings from INI */
	CSimpleIniA config;
	if (config.LoadFile(std::string(useful::ExePath() + std::string("/VisDat_Config.ini")).c_str()) < 0) {
		std::cout << "VisDat_Config.ini not found! Creating file" << std::endl;
		config.SetValue("settings", "title", "CHANGE_ME");
		config.SetValue("settings", "winx", "256");
		config.SetValue("settings", "winy", "256");
		config.SetValue("settings", "scale", "3");
		config.SetValue("settings", "bitdepth", "32");
		config.SetValue("settings", "start", "400000");
		config.SetValue("settings", "textsize", "14");
		config.SetValue("settings", "bUseVexRender", "0");
		config.SetValue("settings", "bFullPageReading", "0");
		if (config.SaveFile(std::string(useful::ExePath() + std::string("/VisDat_Config.ini")).c_str()) < 0) {
			std::cout << "Could not create config" << std::endl;
		}
		else {
			std::cout << "Config saved! Restart VisDat" << std::endl;
		}
		system("pause");
		exit(0);
	}
	
	/* apply settings from ini */
	int cfgWinX = std::stoi(config.GetValue("settings", "winx"));
	int cfgWinY = std::stoi(config.GetValue("settings", "winy"));
	int cfgScale = std::stoi(config.GetValue("settings", "scale"));
	int cfgBitDepth = std::stoi(config.GetValue("settings", "bitdepth"));
	int cfgTextSize = std::stoi(config.GetValue("settings", "textsize"));
	bool cfgVexRender = std::stoi(config.GetValue("settings", "bUseVexRender"));
	bool cfgFullPage = std::stoi(config.GetValue("settings", "bFullPageReading"));

	std::string cfgWinTitle = config.GetValue("settings", "title");
	DWORD cfgStart = 0;
	std::stringstream startss;
	startss << std::hex << config.GetValue("settings", "start");
	startss >> cfgStart;


	/* Start VisDat and render window */
	VisDat vis(cfgWinTitle,cfgScale,cfgWinX,cfgWinY,cfgBitDepth,cfgTextSize,cfgStart,cfgVexRender,cfgFullPage);
	sf::RenderWindow window(sf::VideoMode(vis.sfWinXScaled, vis.sfWinYScaled, vis.sfBitDepth), "VisDatTidy");
	sf::Event event;
	std::stringstream final_ss;

	/* Console Handler */
	CMD con(&vis);

	while (window.isOpen()) {

		/* Handle events */
		while (window.pollEvent(event)) {

			/* Close it when it's being closed lol */
			if (event.type == sf::Event::Closed) {
				window.close();
				CloseHandle(vis.processHandle);
			}

			/* Controls */
			if (event.type == sf::Event::MouseWheelMoved) {
			
				if(sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)){
					vis.cSetWidth(event.mouseWheel.delta);
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt)) {
					vis.cShiftByte(event.mouseWheel.delta);
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Tab)) {
					useful::schift += event.mouseWheel.delta;
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
					if ((event.mouseWheel.delta < 0) && (vis.pthickness + event.mouseWheel.delta) > 0) {
						vis.pthickness += event.mouseWheel.delta;
					}
					else if (event.mouseWheel.delta > 0) {
						vis.pthickness += event.mouseWheel.delta;
					}
				}
				else {
					vis.cScroll(event.mouseWheel.delta, sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
				}
			}

			/* Get Mouse Position */
			if (event.type == sf::Event::MouseMoved) {
				sf::Vector2i v = sf::Mouse::getPosition(window);
				vis.MouseX = v.x / vis.sfScale;
				vis.MouseY = v.y / vis.sfScale;
			}

			/* Handle Mouse Clicks */
			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) || sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle) || sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)){//(event.type == sf::Event::MouseButtonPressed) {
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle)) {
					vis.cPaintDropper();
				}
				else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
					vis.cPaintDraw();
				}
				else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
						vis.cSaveAddressToConfig(&config);
					}
					else {
						vis.cSaveAddressToClipboard();
					}
					
				}
			}

			if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
				vis.firstclick = true;
			}

			
			


		}

		/* 
		/* Handle Command prompt button BROKEN
		if (GetAsyncKeyState('C')) {
			while (GetAsyncKeyState('C')) {
				Sleep(50);
			}
			Sleep(100);
			con.HandleCommand();
		}
		*/



		/* Update status Text */
		final_ss << "Address: 0x" << std::hex << vis.StartAddress << std::endl;
		final_ss << "Width: " << std::dec << vis.sfWinXScaled / vis.sfScale << std::endl;
		final_ss << "mX: " << vis.MouseX << std::endl;
		final_ss << "mY: " << vis.MouseY << std::endl;
		final_ss << "Mouse: 0x" << vis.MouseHover();
		final_ss << "Thick: " << std::dec << vis.pthickness << std::endl;


		/* Draw the things */
		window.clear(sf::Color::Black);
		vis.ReadDataAndDrawOnIMG();
		vis.UpdateText(final_ss.str());
		vis.MouseBox();
		final_ss.str(""); //Clear stream
		if (vis.UseVexRender) {
			window.draw(vis.vex);
			vis.vex.clear();
		}
		else {
			window.draw(vis.sfSpr);
		}
		window.draw(vis.sfText);
		window.draw(vis.MBox);
		window.display();
	}

	return 0;
}