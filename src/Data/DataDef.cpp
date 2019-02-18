#include "DataDef.h"
#include "../Logic/MainLogic.h"

	namespace UI
{
	int PLAYER_NUM = 4;
	int MAX_ROUND = 300;
	int TOWER_NUM = 19;
    QString logFileName = "Log/log.txt";

    QMap<QString, int>TPlayer::LUTPLAYER;
    QMap<QString, int>TTower::LUTTOWER;
    QMap<QString, int>TSoldier::LUTSOLDIER;
    QMap<int, QString>TPlayer::properties;
	UIObject::UIObject(const UIObjectType& uiObjectType, const bool& visiable) :m_nUIType(uiObjectType), m_bVisible(visiable)
	{
		m_bGenerated = false;
	}

	UIObject::~UIObject()
	{
		m_bGenerated = false;
	}

	void UIObject::setVisible(const bool&visible)
	{
		m_bVisible = visible;
	}

	bool UIObject::getVisible()
	{
		return m_bVisible;
	}

	void UIObject::setUIType(const UIObjectType& UIType)
	{
		m_nUIType = UIType;
	}

	UIObjectType UIObject::getUIType()
	{
		return m_nUIType;
	}

	bool UIObject::IsGenerated()
	{
		return m_bGenerated;
	}

	TPlayer::TPlayer() :UIObject(TypeAsPlayer)
	{
        if (LUTPLAYER.empty())
			LUT_INIT();
        if(properties.empty())
            INIT_PROPERTY();
	}

	void TPlayer::LUT_INIT()
	{
		if (LUTPLAYER.empty())
		{
            LUTPLAYER["Rank"] = 0;
            LUTPLAYER["Score"]= 1;
            LUTPLAYER["KillNum"]=2;
            LUTPLAYER["TowerNum"]= 3;
            LUTPLAYER["SurvivalRound"]= 4;
            LUTPLAYER["SoldierNum"]= 5;
            LUTPLAYER["Resource"]= 6;
            LUTPLAYER["MaxPopulation"]= 7;
            LUTPLAYER["Population"]=8;


		}
	}

    void TPlayer::INIT_PROPERTY()
    {
        if(properties.empty())
        {
            properties[0]="Rank";
            properties[1] = "Score";
            properties[2] = "KillNum";
            properties[3] = "SurvivalRound";
            properties[4] = "Resource";
            properties[5] = "MaxPopulation";
            properties[6] = "Population";
            properties[7] = "TowerNum";
            properties[8] = "Towers";
            properties[9] = "SoldierNum";
            properties[10] = "Soldiers";
        }
    }
	TPlayer::~TPlayer()
	{
	}
    void TPlayer::Generate(const std::string &strLine)
	{
        std::string strHolder;
        std::stringstream ssFormat(strLine);
		while (!ssFormat.eof())
		{
			ssFormat >> strHolder;
            switch (LUTPLAYER[QString::fromStdString(strHolder)])
			{
			default:

                break;
			case 0:
				ssFormat >> m_nRank;
				break;
			case 1:
				ssFormat >> m_nScore;
				break;
			case 2:
				ssFormat >> m_nKillNum;
				break;
			case 3:
				ssFormat >> m_nTowerNum;
				break;
			case 4:
				ssFormat >> m_nSurvivalRound;
				break;
			case 5:
				ssFormat >> m_nSoldierNum;
				break;
			case 6:
				ssFormat >> m_nResource;
				break;
			case 7:
				ssFormat >> m_nMaxPopulation;
				break;
			case 8:
				ssFormat >> m_nPopulation;
				break;
			}
		}
		m_bGenerated = true;
        m_vecTowers.clear();
        m_vecrSoldiers.clear();
        m_vecCommands.clear();
        m_strVecCommands.clear();
	}

	void TPlayer::Clear()
	{
	}

	void TPlayer::UIUpdate()
	{
	}

	TTower::TTower() : UIObject(TypeAsTower)
	{
        if (LUTTOWER.empty())
            LUT_INIT();
        m_pOwner = nullptr;
	}


    TTower::TTower(const int& towerID, const int& owner, const int& level, const int& blood, const int& recruiting, const int& recruitingRound, const QString& recruitingType) :UIObject(UIObjectType::TypeAsTower)
	{
		if (LUTTOWER.empty())
			LUT_INIT();
		this->m_nID = towerID;
		if (owner != -1)
        {   try
            {
                this->m_pOwner = MainLogic::GetInstance()->players[owner];
            }
            catch(const std::exception&){this->m_pOwner = nullptr;}
        }
        else
		{
			this->m_pOwner = nullptr;
		}
		this->m_nLevel = level;
		this->m_nBlood = blood;
		this->m_bRecruiting = (recruiting != 0);
		this->m_nRecruitingRound = recruitingRound;
		this->m_nRecruitingType = SoldierTypeStr2Enum(recruitingType);

		this->m_bUpgrade = false;
		this->m_strctProduceSoldier = ProduceType{ false, NoneSoldierType };
        if (LUTTOWER.empty())
            LUT_INIT();
    }


	TTower::~TTower()
    {

	}

	void TTower::LUT_INIT()
	{
		if (LUTTOWER.empty())
		{
            LUTTOWER["TowerID"]= 0;
            LUTTOWER["Owner"] = 1;
            LUTTOWER["Level"] = 2;
            LUTTOWER["Blood"] = 3;
            LUTTOWER["Recruiting"] = 4;
            LUTTOWER["RecruitingRound"] = 5;
            LUTTOWER["RecruitingType"] = 6;
		}
	}

  void TTower::SetVec2Position(const int & id) 
  {
    switch (id)
    {
    case 0:
      this->m_Position = QPointF(6, 89);
      break;
    case 1:
      this->m_Position = QPointF(20, 86);
      break;
    case 2:
      this->m_Position = QPointF(33, 92);
      break;
    case 3:
      this->m_Position = QPointF(44, 77);
      break;
    case 4:
      this->m_Position = QPointF(61, 74);
      break;
    case 5:
      this->m_Position = QPointF(71, 88);
      break;
    case 6:
      this->m_Position = QPointF(83, 77);
      break;
    case 7:
      this->m_Position = QPointF(15, 59);
      break;
    case 8:
      this->m_Position = QPointF(15, 43);
      break;
    case 9:
      this->m_Position = QPointF(28, 51);
      break;
    case 10:
      this->m_Position = QPointF(51, 54);
      break;
    case 11:
      this->m_Position = QPointF(63, 49);
      break;
    case 12:
      this->m_Position = QPointF(73, 46);
      break;
    case 13:
      this->m_Position = QPointF(85, 52);
      break;
    case 14:
      this->m_Position = QPointF(8, 16);
      break;
    case 15:
      this->m_Position = QPointF(22, 21);
      break;
    case 16:
      this->m_Position = QPointF(39, 10);
      break;
    case 17:
      this->m_Position = QPointF(61, 25);
      break;
    case 18:
      this->m_Position = QPointF(77, 15);
      break;
    default:
      this->m_Position = QPointF(0, 0);
    }
  }
  int TTower::getOwnerID()const
  {
	  if (m_pOwner)
		  return m_pOwner->m_nID;
	  else
		  return -1;
  }
    void TTower::Generate(const std::string& strLine)
	{
        if (LUTTOWER.empty())
            LUT_INIT();
        std::string strHolder;
        std::stringstream ssFormat(strLine);
		int temp_int;
        std::string temp_str;
		while (!ssFormat.eof())
		{
			ssFormat >> strHolder;
            switch (LUTTOWER[QString::fromStdString(strHolder)])
			{
			case 0:
				ssFormat >> m_nID; break;
			case 1:
				ssFormat >> temp_int;
				try
				{
					if (temp_int < 0)
						m_pOwner = nullptr;
					else
						m_pOwner = MainLogic::GetInstance()->players[temp_int];
				}
				catch (const std::exception&)
				{
					m_pOwner = nullptr;
				}
				break;
			case 2:
				ssFormat >> m_nLevel;
				break;
			case 3:
				ssFormat >> m_nBlood;
				break;
			case 4:
				ssFormat >> temp_int;
				m_bRecruiting = (temp_int != 0);
				break;
			case 5:
				ssFormat >> m_nRecruitingRound;
				break;
			case 6:
				ssFormat >> temp_str;
				m_nRecruitingType = SoldierTypeStr2Enum(temp_str);
				break;
            default:
				break;
			}
		}
        if(m_pOwner!=nullptr)
        try
        {
            m_pOwner->m_vecTowers.push_back(this);
        }
        catch(const std::exception&){}
    }

	void TTower::Clear()
	{

	}

	void TTower::UIUpdate()
	{
	}

    TSoldier::TSoldier() :UIObject(TypeAsSoldier)
	{
		if (LUTSOLDIER.empty())
			LUT_INIT();
        m_pOwner = nullptr;
	}

    TSoldier::TSoldier(int soldierID, int owner, QString type, int level, int blood, int x_position, int y_position)
        :UIObject(TypeAsSoldier)
	{
		if (LUTSOLDIER.empty())
			LUT_INIT();
		this->m_nID = soldierID;
		try
		{
			if (owner < 0)
				m_pOwner = nullptr;
			else
				this->m_pOwner = MainLogic::GetInstance()->players[owner];
		}
		catch (const std::exception&)
		{
			this->m_pOwner = nullptr;
		}
		this->m_nSoldierType = SoldierTypeStr2Enum(type);
		this->m_nLevel = level;
		this->m_nBlood = blood;
        this->m_Position = QPointF(x_position, y_position);
		this->m_strctSoldierMove = SoldierMoveType{ false, UP, 0 };
		this->m_pVictim = nullptr;
		this->m_bFreshman = true;
		this->m_bDead = (blood <= 0);
	}

	TSoldier::~TSoldier()
	{
	}

	void TSoldier::LUT_INIT()
	{
		if (LUTSOLDIER.empty())
		{
            LUTSOLDIER["SoldierID"] =  0;
            LUTSOLDIER["Owner"]= 1;
            LUTSOLDIER["Type"] = 2;
            LUTSOLDIER["Level"] =3;
            LUTSOLDIER["Blood"]= 4;
            LUTSOLDIER["X_Position"]= 5;
            LUTSOLDIER["Y_Position"]= 6;
		}
	}
    void TSoldier::Generate(const std::string&strLine)
	{
        if(LUTSOLDIER.empty())
            LUT_INIT();
        std::string strHolder;
        std::stringstream ssFormat(strLine);
		int temp_int, x_pos = -1, y_pos = -1;
        std::string temp_str;
		while (!ssFormat.eof())
		{
			ssFormat >> strHolder;
            switch (LUTSOLDIER[QString::fromStdString(strHolder)])
			{
			case 0:
				ssFormat >> m_nID;
				break;
			case 1:
				ssFormat >> temp_int;
				try
				{
					if (temp_int < 0)
						m_pOwner = nullptr;
					else
						m_pOwner = MainLogic::GetInstance()->players[temp_int];
				}
				catch (const std::exception&)
				{
					m_pOwner = nullptr;//This should not happen
				}
				break;
			case 2:
				ssFormat >> temp_str;
				m_nSoldierType = SoldierTypeStr2Enum(temp_str);
				break;
			case 3:
				ssFormat >> m_nLevel;
				break;
			case 4:
				ssFormat >> m_nBlood;
				break;
			case 5:
				ssFormat >> x_pos;
				break;
			case 6:
				ssFormat >> y_pos;
				break;

			default:

				break;
			}
		}

        m_Position = QPointF(x_pos, y_pos);
		if (m_nBlood <= 0)
			m_bDead = true;
        if(m_pOwner!=nullptr)
        try
        {
            m_pOwner->m_vecrSoldiers.push_back(this);
        }
        catch(const std::exception&){}
    }

	void TSoldier::Clear()
	{
	}

	void TSoldier::UIUpdate()
	{
	}

	int TSoldier::Info2GID()
	{
		return SOLDIER_SET_START + ((m_pOwner->m_nID + 1) * SOLDIER_SET_COL + m_nSoldierType - 1);
	}
	int TSoldier::getOwnerID()const
	{
		if (m_pOwner)
			return m_pOwner->m_nID;
		else
			return -1;
	}

    QString TSoldier::getImageName()
    {
        try
        {
            return SoldierTypeEnum2Str(this->m_nSoldierType)
                +QString::number(this->m_pOwner->m_nID)+".png";
        }
        catch(const std::exception&)
        {
            return "";
        }
    }
	Command::Command()
	{}

	Command::~Command()
	{}






    SoldierType SoldierTypeStr2Enum(QString str)
	{
		if (str == "LightInfantry")
			return LightInfantry;
		else if (str == "LightArcher")
			return LightArcher;
		else if (str == "LightKnight")
			return LightKnight;
		else if (str == "Mangonel")
			return Mangonel;
		else if (str == "HeavyInfantry")
			return HeavyInfantry;
		else if (str == "HeavyArcher")
			return HeavyArcher;
		else if (str == "HeavyKnight")
			return HeavyKnight;
		else
			return NoneSoldierType;
	}
    SoldierType SoldierTypeStr2Enum(std::string str)
    {
        if (str == "LightInfantry")
            return LightInfantry;
        else if (str == "LightArcher")
            return LightArcher;
        else if (str == "LightKnight")
            return LightKnight;
        else if (str == "Mangonel")
            return Mangonel;
        else if (str == "HeavyInfantry")
            return HeavyInfantry;
        else if (str == "HeavyArcher")
            return HeavyArcher;
        else if (str == "HeavyKnight")
            return HeavyKnight;
        else
            return NoneSoldierType;
    }

    QString SoldierTypeEnum2Str(const SoldierType &soldiertype)
    {
        switch(soldiertype)
        {
        case LightInfantry:
            return "LightInfantry";
        case LightArcher:
            return "LightArcher";
        case LightKnight:
            return "LightKnight";
        case Mangonel:
            return "Mangonel";
        case HeavyInfantry:
            return "HeavyInfantry";
        case HeavyArcher:
            return "HeavyArcher";
        case HeavyKnight:
            return "HeavyKnight";
        default:
            return "";
        }
    }

    MoveDirection moveDirStr2Enum(QString str)
	{
		if (str == "UP")
			return MoveDirection::UP;
		else if (str == "DOWN")
			return MoveDirection::DOWN;
		else if (str == "LEFT")
			return MoveDirection::LEFT;
		else //if (str == "RIGHT")
			return MoveDirection::RIGHT;
	}
    MoveDirection moveDirStr2Enum(std::string str)
    {
        if (str == "UP")
            return MoveDirection::UP;
        else if (str == "DOWN")
            return MoveDirection::DOWN;
        else if (str == "LEFT")
            return MoveDirection::LEFT;
        else //if (str == "RIGHT")
            return MoveDirection::RIGHT;
    }

    void MyClear(std::stringstream & ifs)
	{
		ifs.clear();
		int last = 0;
		while (ifs.tellg())
		{
			ifs.seekg(last + 1);
			if (int(ifs.tellg()) == -1)
				break;
			last++;
		}
		ifs.clear();
		ifs.seekg(last);

	}

}
