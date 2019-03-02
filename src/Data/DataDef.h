#pragma once

#include <QMap>
#include <QVector>
#include <fstream>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <string>
#include <sstream>
#include <tchar.h>
#include <windows.h>
#include <QException>
#include <QPointF>
#include <QObject>
#include <QDebug>
#define SOLDIER_SET_START 17
#define SOLDIER_SET_COL 7

namespace UI
{
    extern QString logFileName;
	extern int MAX_ROUND;
	extern int PLAYER_NUM;
	extern int TOWER_NUM;


	enum UIObjectType
	{
		TypeAsNone,
		TypeAsPlayer,
		TypeAsTower,
		TypeAsSoldier
	};

	enum MoveDirection
	{
		UP = 1
		, DOWN = 2
		, LEFT = 3
		, RIGHT = 4
	};
    MoveDirection moveDirStr2Enum(QString str);
    MoveDirection moveDirStr2Enum(std::string str);


	enum SoldierType
	{
        LightInfantry = 1
        , LightArcher = 2
        , LightKnight = 3
        , Mangonel = 4
        , HeavyInfantry = 5
        , HeavyArcher = 6
        , HeavyKnight = 7
		, NoneSoldierType
	};
    SoldierType SoldierTypeStr2Enum(QString str);
    SoldierType SoldierTypeStr2Enum(std::string str);
    QString SoldierTypeEnum2Str(const SoldierType& soldiertype);

	struct SoldierMoveType
	{
		bool m_bMove;
		MoveDirection m_nMoveDirection;
		int m_nMoveDistance;
	};

	struct ProduceType
	{
		bool m_bProduce;
		SoldierType m_nSoldierType;
	};

	enum CommandType
	{
		Attack       //(SoldierID, target_x, target_y)
		, Move       //(SoldierID, moveDirection, moveDistance)
		, Upgrade    //(TowerID)
		, Produce    //(TowerID, SoldierType)
	};

	enum TLandForm
	{
		Road,
		Forest,
		River,
		Mountain,
		Dorm,
		Classroom
	};

	class UIObject
	{
	public:
		UIObjectType m_nUIType;
		bool m_bVisible;
		bool m_bGenerated;

	public:
		//Constructor & Destructor
		UIObject(const UIObjectType& UIType = TypeAsNone, const bool& visible = true);

		virtual ~UIObject();
		//	Properties
		void setVisible(const bool&);
		bool getVisible();
		void setUIType(const UIObjectType&);
		UIObjectType getUIType();
		bool IsGenerated();

		//	Interfaces
        virtual void Generate(const std::string&) = 0;
		virtual void Clear() = 0;
		virtual void UIUpdate() = 0;
		virtual void LUT_INIT() = 0;

	};

	class TPlayer;
	class TTower;
	class TSoldier;
    class Command;

	class TPlayer : public UIObject
	{
    private:
        static QMap<QString, int>LUTPLAYER;
    public:
        static QMap<int, QString>properties;
        void INIT_PROPERTY();
    public:
		int m_nID;
        QVector<TTower*>m_vecTowers;
        QVector<TSoldier*>m_vecrSoldiers;
        QVector<Command*>m_vecCommands;
        QMap<Command*,QString>m_strVecCommands;
		int m_nRank;
		int m_nResource;
		int m_nScore;
		int m_nKillNum;
		int m_nTowerNum;
		int m_nSurvivalRound;
		int m_nMaxPopulation;
		int m_nPopulation;
		int m_nSoldierNum;

		TPlayer();
		virtual ~TPlayer();
        virtual void Generate(const std::string&);
		virtual void Clear();
		virtual void UIUpdate();
		virtual void LUT_INIT();
	};

    class TSoldier : public UIObject
	{

	private:
        static QMap<QString, int>LUTSOLDIER;
	public:
		int m_nID;
		TPlayer* m_pOwner;	
		int m_nBlood;
		SoldierType m_nSoldierType;
        QPointF m_Position;
		int m_nLevel;
		SoldierMoveType m_strctSoldierMove;
		UIObject* m_pVictim;
		bool m_bFreshman;
		bool m_bDead;
        bool m_bAnimation;

		//	Default Constructor
		TSoldier();
		//	Specified constructor, use this to contruct new soldier from result.txt
        TSoldier(int soldierID, int owner, QString type, int level, int blood, int x_position, int y_position);
		~TSoldier();
        virtual void Generate(const std::string&strLine);
		virtual void Clear();
		virtual void UIUpdate();
		virtual void LUT_INIT();
		int Info2GID();
		int getOwnerID()const;
        QString getImageName();
	};



	class TTower : public UIObject
	{
	private:
        static QMap<QString, int>LUTTOWER;
	public:

		ProduceType m_strctProduceSoldier;
		int m_nID;
		TPlayer* m_pOwner;
		int m_nLevel;
		int m_nBlood;

		bool m_bRecruiting;
		SoldierType m_nRecruitingType;
		int m_nRecruitingRound;
		bool m_bUpgrade; //


        QPointF m_Position;

		TTower();
        TTower(const int& towerID, const int& owner, const int& level, const int& blood, const int& recruiting, const int& recruitingRound, const QString& recruitingType);
		~TTower();
        virtual void Generate(const std::string&strLine);
		virtual void Clear();
		virtual void UIUpdate();
		virtual void LUT_INIT();
		void SetVec2Position(const int& id);
		int getOwnerID()const;
	};



	class Command
	{


	public:


		CommandType m_nCommandType;
		TPlayer* m_pOwner;

		//Move
		TSoldier* m_pMoveSoldier;
		MoveDirection m_nMoveDirection;
		int m_nMoveDistance;

		//Attack
		TSoldier* m_pAttackObject;
		UIObject* m_pVictimObject;

		//Upgrade
		TTower* m_pUpgradeTower;

		//Produce
		TTower* m_pProduceTower;
		SoldierType m_nProduceSoldierType;


		Command();
		~Command();
	};

    void MyClear(std::stringstream& ifs);
	void resetDirectory();
	template<class T1, class T2>
    T1 maxKey(const QMap<T1, T2>&m)
	{
        if(m.empty())
            return -1;
        else
        {
            return *(m.keys().cend());
        }
    }

}
