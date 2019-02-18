#pragma once

#include "../Data/DataDef.h"
#include "../UI/StartScene.h"

class PlayScene;
namespace UI {
    class MainLogic
	{
		enum GameState
		{
			GAME_NOT_START,
			GAME_RUNNING,
			GAME_PAUSE
		};
	public:
		GameState gameState;
		int gameRound;
		int playerAlive;
		float speed;
	public:
		std::ofstream logFileStream;

        QString loadFileName;
		std::ifstream ifsGameResult;
        QVector<UI::TTower*>towers;
        QMap<int, UI::TSoldier*>soldiers;
        QVector<UI::TPlayer*>players;
        QVector<UI::Command*>commands;
        QString mapFile;
	public:



		MainLogic();
        ~MainLogic();

		void GameStart();
		void GameLoop();
		void GameOver();
		void GamePause();
		void GameResume();


		void LoadData();
		bool LogicUpdate();
        void parseLines(const std::string& mark_type, const int& mark_lines);

		void clearData();
		void initData();
        void WriteLog(QString& message);
        void WriteLog(const std::string& message);
		void WriteInfo(const UI::TPlayer*player);
		void WriteInfo(const UI::TSoldier*soldier);
		void WriteInfo(const UI::TTower*tower);
		void WriteInfo(const UI::Command*command);

    public:
        StartScene* startScene;
        PlayScene* playScene;
		//Singleton.
	public:
		static MainLogic* m_pInstance;
		static MainLogic* GetInstance();
		static void DestroyInstance();
	};

}
