#pragma once
#include <BWAPI.h>
#include <time.h>
#include <conio.h>
#include "TreeNode.h"
#include "DecisionTree.h"

#define MAX_UNIT_COUNT		1000
#define MAX_SCV				19 // 2 workers per minerals (9 minerals = 18 workers), 3 for gas, 1 for building structures
#define MAX_MISSILE_TURRET	13 // If player in front use wraiths or other air units
#define MAX_MARINE			20
#define MAX_VULTURE			20

#define MAX_UNIT_COUNT 1000

using namespace BWAPI;
using namespace Filter;

const int ROOT = 1;
const int IS_REFINERY = 2;
const int IS_BARRACKS = 3;
const int IS_FACTORY = 4;
const int IS_MACHINESHOP = 5;
const int BUILDBARRACK = 6;
const int BUILDFACTORY = 7;
const int BUILDREFINERY = 8;
const int BUILDMACHINESHOP = 9;

const int DONE = 10;

class BigAI : public BWAPI::AIModule
{
public:
  virtual void onStart();
  virtual void onEnd(bool isWinner);
  virtual void onFrame();
  virtual void onUnitCreate(Unit unit);
  virtual void onUnitDestroy(Unit unit);
  virtual void onUnitComplete(Unit unit);

  void constructBuildings();
  void trainUnits();

  void InitBuildTree();
  void ExecuteBuildTree();

  static void addUnit(Unit unitSet[], Unit u)
  {
	  for (int i = 0; i < MAX_UNIT_COUNT; i++)
		  if (unitSet[i] == nullptr)
		  {
			  unitSet[i] = u;
			  break;
		  }
  }

  static void deleteUnit(Unit unitSet[], Unit u)
  {
	  for (int i = 0; i < MAX_UNIT_COUNT; i++)
		  if (unitSet[i] == u)
		  {
			  unitSet[i] = nullptr;
			  break;
		  }
  }

private:
	TilePosition _enemyTilePosition;
	TilePosition _commandCenterTilePosition;
	TilePosition _randomTilePosition;

	Position _commandCenterPosition;
	Position _enemyPosition;
	Position _randomPosition;

	Unit m_pSCVs[MAX_UNIT_COUNT];
	Unit m_pMarine[MAX_UNIT_COUNT];
	Unit m_pVulture[MAX_UNIT_COUNT];
	Unit m_pTank[MAX_UNIT_COUNT];

	Unit m_pMineral[MAX_UNIT_COUNT];
	Unit m_pGas[MAX_UNIT_COUNT];

	Unit m_CommandCenter;
	Unit m_pRefinery[MAX_UNIT_COUNT];
	Unit m_pSupplyDepot[MAX_UNIT_COUNT];
	Unit m_pBarracks[MAX_UNIT_COUNT];
	Unit m_pAcademy[MAX_UNIT_COUNT];
	Unit m_pFactory[MAX_UNIT_COUNT];
	Unit m_pArmory[MAX_UNIT_COUNT];
	Unit m_pBunker[MAX_UNIT_COUNT];
	Unit m_pEngineeringBay[MAX_UNIT_COUNT];
	Unit m_pMissile_Turret[MAX_UNIT_COUNT];
	Unit m_pStarport[MAX_UNIT_COUNT];
	Unit m_pScienceFacility[MAX_UNIT_COUNT];
	Unit m_pMachineShop[MAX_UNIT_COUNT];

	Unit m_pPowerGenerator[MAX_UNIT_COUNT];


	int m_nSCVCounter;
	int m_nMarineCounter;
	int m_nVultureCounter;
	int m_nTankCounter;

	int m_nGasCounter;
	int m_nBarrackCounter;
	int m_nSupplyCounter;
	int m_nRefineryCounter;
	int m_nAcademyCounter;
	int m_nFactoryCounter;
	int m_nArmoryCounter;
	int m_nBunkerCounter;
	int m_nEngineeringBayCounter;
	int m_nMissileTurretCounter;
	int m_nStarportCounter;
	int m_nScienceFacilityCounter;
	int m_nMachineShopCounter;

	int m_nPowerGenerator;

	int m_nEnemyCommandCenterCounter;

	DecisionTree* m_buildTree;
	bool m_bcheckBuild;
	int m_iBuildCondition;
};
