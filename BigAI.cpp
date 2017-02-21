#include "BigAI.h"
#include <iostream>
#include <time.h>

using namespace BWAPI;
using namespace Filter;

void BigAI::onStart()
{
	Broodwar->sendText("BigAI here!");
	Broodwar << "The map is " << Broodwar->mapName() << "!" << std::endl;
	Broodwar->setCommandOptimizationLevel(2);

	for (int i = 0; i < MAX_UNIT_COUNT; i++)
	{
		// Units array
		m_pSCVs[i] = nullptr;
		m_pMineral[i] = nullptr;
		m_pGas[i] = nullptr;
		m_pTank[i] = nullptr;

		// Buildings array
		m_pRefinery[i] = nullptr;
		m_pBarracks[i] = nullptr;
		m_pAcademy[i] = nullptr;
		m_pArmory[i] = nullptr;
		m_pBunker[i] = nullptr;
		m_pEngineeringBay[i] = nullptr;
		m_pFactory[i] = nullptr;
		m_pMissile_Turret[i] = nullptr;
		m_pStarport[i] = nullptr;
		m_pScienceFacility[i] = nullptr;
		m_pRefinery[i] = nullptr;
		m_pSupplyDepot[i] = nullptr;
		m_pMachineShop[i] = nullptr;

		m_pPowerGenerator[i] = nullptr;
	}

	// Units counters
	m_nSCVCounter = 0;
	m_nMarineCounter = 0;
	m_nVultureCounter = 0;
	m_nTankCounter = 0;

	// Buildings counters
	m_nBarrackCounter = 0;
	m_nAcademyCounter = 0;
	m_nArmoryCounter = 0;
	m_nBunkerCounter = 0;
	m_nEngineeringBayCounter = 0;
	m_nFactoryCounter = 0;
	m_nMissileTurretCounter = 0;
	m_nRefineryCounter = 0;
	m_nScienceFacilityCounter = 0;
	m_nStarportCounter = 0;
	m_nSupplyCounter = 0;
	m_nMachineShopCounter = 0;

	m_nPowerGenerator = 0;

	m_nEnemyCommandCenterCounter = 1;

	// MAP 1 : Start location south east = x:53, y:55
	// MAP 2 : Start location south west = x:7, y:55  ---  Start location south est = x:53, y:55
	TilePosition::list startLocations = Broodwar->getStartLocations();
	_commandCenterTilePosition = Broodwar->self()->getStartLocation();
	//TilePosition myPosition = *(startLocations.begin());
	for (std::deque<TilePosition>::iterator it = startLocations.begin(); it != startLocations.end(); ++it)
	{
		//Broodwar << "Position X : " << it->x << " Position Y : " << it->y << std::endl;
		if (it->x != _commandCenterTilePosition.x)
		{
			_enemyTilePosition.x = it->x;
			_enemyTilePosition.y = it->y;
		}
	}
	// Cast TilePosition into Position for attack() function
	_enemyPosition = Position(_enemyTilePosition);
	_commandCenterPosition = Position(_commandCenterTilePosition);

	_randomTilePosition.x = _enemyTilePosition.x;
	_randomTilePosition.y = _enemyTilePosition.y - 50;
	_randomPosition = Position(_randomTilePosition);

	InitBuildTree();
}

void BigAI::onEnd(bool isWinner)
{
	Broodwar << "gg" << std::endl;
	delete m_buildTree;
}

void BigAI::InitBuildTree()
{
	m_buildTree = new DecisionTree();

	m_buildTree->CreateRootNode(1, ROOT);
	m_buildTree->AddLeftNode(1, 2, IS_BARRACKS);

	m_buildTree->AddLeftNode(2, 4, IS_REFINERY);
	m_buildTree->AddRightNode(2, 5, BUILDBARRACK);

	m_buildTree->AddLeftNode(4, 8, IS_FACTORY);
	m_buildTree->AddRightNode(4, 9, BUILDREFINERY);

	m_buildTree->AddLeftNode(8, 12, IS_MACHINESHOP);
	m_buildTree->AddRightNode(8, 13, BUILDFACTORY);

	m_buildTree->AddLeftNode(12, 17, DONE);
	m_buildTree->AddRightNode(12, 18, BUILDMACHINESHOP);

	m_buildTree->AddLeftNode(18, 25, DONE);

	m_buildTree->AddLeftNode(13, 19, IS_MACHINESHOP);

	m_buildTree->AddLeftNode(19, 26, DONE);
	m_buildTree->AddRightNode(19, 27, BUILDMACHINESHOP);

	m_buildTree->AddLeftNode(27, 34, DONE);

	m_buildTree->AddLeftNode(9, 14, IS_FACTORY);

	m_buildTree->AddLeftNode(14, 20, IS_MACHINESHOP);
	m_buildTree->AddRightNode(14, 21, BUILDFACTORY);

	m_buildTree->AddLeftNode(20, 28, DONE);
	m_buildTree->AddRightNode(20, 29, BUILDMACHINESHOP);

	m_buildTree->AddLeftNode(29, 35, DONE);

	m_buildTree->AddLeftNode(21, 30, IS_MACHINESHOP);

	m_buildTree->AddLeftNode(30, 36, DONE);
	m_buildTree->AddRightNode(30, 37, BUILDMACHINESHOP);

	m_buildTree->AddLeftNode(37, 39, DONE);

	m_buildTree->AddLeftNode(5, 10, IS_FACTORY);

	m_buildTree->AddLeftNode(10, 15, IS_MACHINESHOP);
	m_buildTree->AddRightNode(10, 16, BUILDFACTORY);

	m_buildTree->AddLeftNode(16, 24, IS_MACHINESHOP);

	m_buildTree->AddLeftNode(24, 32, DONE);
	m_buildTree->AddRightNode(24, 33, BUILDMACHINESHOP);

	m_buildTree->AddLeftNode(33, 38, DONE);


	m_bcheckBuild = false;
	m_iBuildCondition = -1;
}

void BigAI::onFrame()
{
	Broodwar->drawTextScreen(200, 0, "FPS: %d", Broodwar->getFPS());
	Broodwar->drawTextScreen(200, 20, "Average FPS: %f", Broodwar->getAverageFPS());

	//Broodwar->drawTextScreen(200, 40, "My start location X: %d, Y: %d", Broodwar->self()->getStartLocation().x, Broodwar->self()->getStartLocation().y);
	//Broodwar->drawTextScreen(200, 60, "Enemy start location X: %d, Y: %d", _enemyTilePosition.x, _enemyTilePosition.y);

	//Broodwar->drawTextScreen(200, 80, "SCV counter = %d", m_nSCVCounter);

	// Return if the game is a replay or is paused
	if (Broodwar->isReplay() || Broodwar->isPaused() || !Broodwar->self())
		return;

	// Prevent spamming by only running our onFrame once every number of latency frames.
	// Latency frames are the number of frames before commands are processed.
	if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0)
		return;

	ExecuteBuildTree();
	trainUnits();
}

void BigAI::constructBuildings()
{
	// Find a location for barrack and build it
	if (m_pSCVs[12] != nullptr && m_pSCVs[12]->isIdle() && m_nBarrackCounter < 1 && Broodwar->self()->minerals() >= UnitTypes::Terran_Barracks.mineralPrice())
	{
		if (m_pSCVs[12]->canBuild(UnitTypes::Terran_Barracks, TilePosition(m_CommandCenter->getTilePosition().x, m_CommandCenter->getTilePosition().y - 4)))
			if (m_nBarrackCounter < 1)
				m_pSCVs[12]->build(UnitTypes::Terran_Barracks, TilePosition(m_CommandCenter->getTilePosition().x, m_CommandCenter->getTilePosition().y - 4));
	}
	else if (m_pSCVs[12] != nullptr && m_pSCVs[12]->isIdle() && m_nBarrackCounter == 1 && m_nSupplyCounter == 0 && Broodwar->self()->minerals() >= UnitTypes::Terran_Supply_Depot.mineralPrice())
	{
		if (m_pSCVs[12]->canBuild(UnitTypes::Terran_Supply_Depot, TilePosition(m_CommandCenter->getTilePosition().x, m_CommandCenter->getTilePosition().y - 6)))
			m_pSCVs[12]->build(UnitTypes::Terran_Supply_Depot, TilePosition(m_CommandCenter->getTilePosition().x, m_CommandCenter->getTilePosition().y - 6));
	}
	// Find a location for Factory and build it
	else if (m_pSCVs[12] != nullptr && m_pSCVs[12]->isIdle() && m_nSupplyCounter >= 1 && m_nFactoryCounter < 1 && m_nBarrackCounter >= 1
		&& Broodwar->self()->minerals() >= UnitTypes::Terran_Factory.mineralPrice())
	{
		if (m_pSCVs[12]->canBuild(UnitTypes::Terran_Factory, TilePosition(m_CommandCenter->getTilePosition().x, m_CommandCenter->getTilePosition().y - 10)))
			m_pSCVs[12]->build(UnitTypes::Terran_Factory, TilePosition(m_CommandCenter->getTilePosition().x, m_CommandCenter->getTilePosition().y - 10));
	}
	// If current supply + 10 is more than total supply, build another supply (basic supply is doubled because of zergling which cost 0.5 and comes by two, so +10 = +5 in real) 
	else if (m_pSCVs[12] != nullptr && m_pSCVs[12]->isIdle() && Broodwar->self()->supplyUsed() + 10 >= Broodwar->self()->supplyTotal()
		&& Broodwar->self()->minerals() >= UnitTypes::Terran_Supply_Depot.mineralPrice())
	{
		// Build supply depots
		TilePosition supplyPosition = Broodwar->getBuildLocation(UnitTypes::Terran_Supply_Depot, m_CommandCenter->getTilePosition(), 25);
		if (m_pSCVs[12]->canBuild(UnitTypes::Terran_Supply_Depot, supplyPosition))
			m_pSCVs[12]->build(UnitTypes::Terran_Supply_Depot, supplyPosition);
	}
}

void BigAI::ExecuteBuildTree()
{
	if (m_buildTree->IsLastLeafNode() == true)
		return;
	constructBuildings();
	return;
	if (m_bcheckBuild == false)
		switch (m_buildTree->Execute(m_iBuildCondition))
		{
		case IS_BARRACKS:
		{
			m_iBuildCondition = 0;
			for (auto u : Broodwar->self()->getUnits())
				if (u != nullptr)
					if (u->getType() == UnitTypes::Terran_Barracks)
						m_iBuildCondition = 1;
			break;
		}
		case IS_FACTORY:
		{
			m_iBuildCondition = 0;
			for (auto u : Broodwar->self()->getUnits())
				if (u != nullptr)
					if (u->getType() == UnitTypes::Terran_Factory)
						m_iBuildCondition = 1;
			break;
		}
		case IS_REFINERY:
		{
			m_iBuildCondition = 0;
			for (auto u : Broodwar->self()->getUnits())
				if (u != nullptr)
					if (u->getType() == UnitTypes::Terran_Refinery)
						m_iBuildCondition = 1;
			break;
		}
		case IS_MACHINESHOP:
		{
			m_iBuildCondition = 0;
			for (auto u : Broodwar->self()->getUnits())
				if (u != nullptr)
					if (u->getType() == UnitTypes::Terran_Machine_Shop)
						m_iBuildCondition = 1;
			break;
		}
		case BUILDBARRACK:
		{
			if (m_pSCVs[12] != nullptr && m_pSCVs[12]->isIdle() && m_nBarrackCounter < 1 && Broodwar->self()->minerals() >= UnitTypes::Terran_Barracks.mineralPrice())
			{
				if (m_pSCVs[12]->canBuild(UnitTypes::Terran_Barracks, TilePosition(m_CommandCenter->getTilePosition().x, m_CommandCenter->getTilePosition().y - 4)))
					if (m_nBarrackCounter < 1)
						m_pSCVs[12]->build(UnitTypes::Terran_Barracks, TilePosition(m_CommandCenter->getTilePosition().x, m_CommandCenter->getTilePosition().y - 4));
			}
			m_bcheckBuild = true;
			m_iBuildCondition = -1;
			break;
		}
		case BUILDFACTORY:
		{
			if (m_pSCVs[12] != nullptr && m_pSCVs[12]->isIdle() && m_nSupplyCounter >= 1 && m_nFactoryCounter < 1 && m_nBarrackCounter >= 1
				&& Broodwar->self()->minerals() >= UnitTypes::Terran_Factory.mineralPrice())
			{
				if (m_pSCVs[12]->canBuild(UnitTypes::Terran_Factory, TilePosition(m_CommandCenter->getTilePosition().x, m_CommandCenter->getTilePosition().y - 10)))
					m_pSCVs[12]->build(UnitTypes::Terran_Factory, TilePosition(m_CommandCenter->getTilePosition().x, m_CommandCenter->getTilePosition().y - 10));
			}
			m_bcheckBuild = true;
			m_iBuildCondition = -1;
			break;
		}
		case BUILDREFINERY:
		{
			if (m_pSCVs[11] != nullptr && m_nRefineryCounter == 0)
			{
				TilePosition refineryPosition = Broodwar->getBuildLocation(UnitTypes::Terran_Refinery, m_pSCVs[11]->getTilePosition());
				if (m_pSCVs[11]->canBuild(UnitTypes::Terran_Refinery, refineryPosition))
					m_pSCVs[11]->build(UnitTypes::Terran_Refinery, refineryPosition);
			}
			if (m_pSCVs[10] != nullptr && m_nRefineryCounter < 2)
			{
				TilePosition refineryPosition = Broodwar->getBuildLocation(UnitTypes::Terran_Refinery, m_pSCVs[10]->getTilePosition());
				if (m_pSCVs[10]->canBuild(UnitTypes::Terran_Refinery, refineryPosition))
					m_pSCVs[10]->build(UnitTypes::Terran_Refinery, refineryPosition);
			}
			m_bcheckBuild = true;
			m_iBuildCondition = -1;
			break;
		}
		case BUILDMACHINESHOP:
		{
			for (auto u : Broodwar->self()->getUnits()) {
				if (u != nullptr && u->getType() == UnitTypes::Terran_Machine_Shop && u->isIdle())
				{
					if (m_nMachineShopCounter == 0)
					{
						u->buildAddon(UnitTypes::Terran_Machine_Shop);
					}
				}
			}
			m_bcheckBuild = true;
			m_iBuildCondition = -1;
			break;
		}
		case DONE:
			Broodwar << "Done" << std::endl;
			break;
		default:
			break;
		}
	else
		Broodwar << "LE ELSE !" << std::endl;
		switch (m_buildTree->GetCurrentNode())
		{
		case BUILDBARRACK:
		{
			if (m_pSCVs[12] != nullptr && m_pSCVs[12]->isIdle() && m_nBarrackCounter < 1 && Broodwar->self()->minerals() >= UnitTypes::Terran_Barracks.mineralPrice())
			{
				if (m_pSCVs[12]->canBuild(UnitTypes::Terran_Barracks, TilePosition(m_CommandCenter->getTilePosition().x, m_CommandCenter->getTilePosition().y - 4)))
					if (m_nBarrackCounter < 1)
						m_pSCVs[12]->build(UnitTypes::Terran_Barracks, TilePosition(m_CommandCenter->getTilePosition().x, m_CommandCenter->getTilePosition().y - 4));
			}
			break;
		}
		case BUILDFACTORY:
		{
			if (m_pSCVs[12] != nullptr && m_pSCVs[12]->isIdle() && m_nSupplyCounter >= 1 && m_nFactoryCounter < 1 && m_nBarrackCounter >= 1
				&& Broodwar->self()->minerals() >= UnitTypes::Terran_Factory.mineralPrice())
			{
				if (m_pSCVs[12]->canBuild(UnitTypes::Terran_Factory, TilePosition(m_CommandCenter->getTilePosition().x, m_CommandCenter->getTilePosition().y - 10)))
					m_pSCVs[12]->build(UnitTypes::Terran_Factory, TilePosition(m_CommandCenter->getTilePosition().x, m_CommandCenter->getTilePosition().y - 10));
			}
			break;
		}
		case BUILDREFINERY:
		{
			if (m_pSCVs[11] != nullptr && m_nRefineryCounter == 0)
			{
				TilePosition refineryPosition = Broodwar->getBuildLocation(UnitTypes::Terran_Refinery, m_pSCVs[11]->getTilePosition());
				if (m_pSCVs[11]->canBuild(UnitTypes::Terran_Refinery, refineryPosition))
					m_pSCVs[11]->build(UnitTypes::Terran_Refinery, refineryPosition);
			}
			if (m_pSCVs[10] != nullptr && m_nRefineryCounter < 2)
			{
				TilePosition refineryPosition = Broodwar->getBuildLocation(UnitTypes::Terran_Refinery, m_pSCVs[10]->getTilePosition());
				if (m_pSCVs[10]->canBuild(UnitTypes::Terran_Refinery, refineryPosition))
					m_pSCVs[10]->build(UnitTypes::Terran_Refinery, refineryPosition);
			}
			break;
		}
		case BUILDMACHINESHOP:
		{
			for (auto u : Broodwar->self()->getUnits()) {
				if (u != nullptr && u->getType() == UnitTypes::Terran_Machine_Shop && u->isIdle())
				{
					if (m_nMachineShopCounter == 0)
					{
						u->buildAddon(UnitTypes::Terran_Machine_Shop);
					}
				}
			}
			break;
		}
		default:
			break;
		}
}

void BigAI::trainUnits()
{
	for (auto &u : Broodwar->self()->getUnits())
	{
		if (!u->exists() && (u->isLockedDown() || u->isMaelstrommed() || u->isStasised()) && (u->isLoaded() || !u->isPowered() || u->isStuck()) && (!u->isCompleted() || u->isConstructing()))
			continue;

		// If it's a worker, send it to gather
		if (u->getType().isWorker())
		{
			if (m_pSCVs[11] != nullptr && m_nRefineryCounter == 0)
			{
				TilePosition refineryPosition = Broodwar->getBuildLocation(UnitTypes::Terran_Refinery, m_pSCVs[11]->getTilePosition());
				if (m_pSCVs[11]->canBuild(UnitTypes::Terran_Refinery, refineryPosition))
					m_pSCVs[11]->build(UnitTypes::Terran_Refinery, refineryPosition);
			}
			if (m_pSCVs[10] != nullptr && m_nRefineryCounter < 2)
			{
				TilePosition refineryPosition = Broodwar->getBuildLocation(UnitTypes::Terran_Refinery, m_pSCVs[10]->getTilePosition());
				if (m_pSCVs[10]->canBuild(UnitTypes::Terran_Refinery, refineryPosition))
					m_pSCVs[10]->build(UnitTypes::Terran_Refinery, refineryPosition);
			}
			if (m_pSCVs[12] != nullptr && m_pSCVs[1] != nullptr && u != m_pSCVs[12] && u != m_pSCVs[1] && u->isIdle())
			{
				if (u->isCarryingGas() || u->isCarryingMinerals())
					u->returnCargo();
				else
					u->gather(u->getClosestUnit(IsMineralField || IsRefinery));
			}
		}
		// If it's our command center & is Idle
		else if (u->getType().isResourceDepot() && u->isIdle())
		{
			if (m_nSCVCounter < MAX_SCV)
			{
				u->train(u->getType().getRace().getWorker());
			}
		}
		// If it's a factory, train vultures
		else if (u->getType() == UnitTypes::Terran_Factory && u->isIdle())
		{
			//u->train(UnitTypes::Terran_Vulture);
			if (m_nMachineShopCounter == 0)
			{
				u->buildAddon(UnitTypes::Terran_Machine_Shop);
			}
			else
			{
				if (m_pMachineShop[0] != nullptr && m_pMachineShop[0]->canResearch(TechTypes::Tank_Siege_Mode))
					m_pMachineShop[0]->research(TechTypes::Tank_Siege_Mode);
				if (m_nTankCounter < 10)
				{
					u->train(UnitTypes::Terran_Siege_Tank_Tank_Mode);
				}
			}
		}
		/*else if (u->getType() == UnitTypes::Terran_Machine_Shop && u->isIdle())
		{
			if (u->canResearch(TechTypes::Tank_Siege_Mode))
				u->research(TechTypes::Tank_Siege_Mode);
		}*/
		// If there are air units in front, train marines && build engineering bay then missiles
		/*if (u->isFlying() && u->getPlayer() != Broodwar->self())
		{
			// Build Engineering Bay
			if (m_nBarrackCounter != 0 && m_nEngineeringBayCounter == 0 && m_pSCVs[0] != nullptr && m_pSCVs[0]->isIdle() && Broodwar->self()->minerals() >= UnitTypes::Terran_Missile_Turret.mineralPrice())
			{
				TilePosition engineeringBayPosition = Broodwar->getBuildLocation(UnitTypes::Terran_Missile_Turret, m_CommandCenter->getTilePosition());
				if (m_pSCVs[0]->canBuild(UnitTypes::Terran_Missile_Turret, engineeringBayPosition))
					m_pSCVs[0]->build(UnitTypes::Terran_Missile_Turret, engineeringBayPosition);
			}
			// Build Missile turrets
			if (m_nEngineeringBayCounter != 0 && m_nMissileTurretCounter < 2 && m_pSCVs[0] != nullptr && m_pSCVs[0]->isIdle() && Broodwar->self()->minerals() >= UnitTypes::Terran_Missile_Turret.mineralPrice())
			{
				TilePosition missilePosition = Broodwar->getBuildLocation(UnitTypes::Terran_Missile_Turret, m_pFactory[0]->getTilePosition(), 10);
				if (m_pSCVs[0]->canBuild(UnitTypes::Terran_Missile_Turret, missilePosition))
					m_pSCVs[0]->build(UnitTypes::Terran_Missile_Turret, missilePosition);
			}
			if (m_pBarracks[0] != nullptr && m_pBarracks[0]->isIdle())
				m_pBarracks[0]->train(UnitTypes::Terran_Marine);
		}*/
		// If there are bunkers in front, train ...
		/*if (u->getType() == UnitTypes::Terran_Bunker)
		{
			/// ...
		}*/
		// If it's a vulture, attack
		if (u->getType() == UnitTypes::Terran_Vulture && u->isIdle() && u->getPlayer() == Broodwar->self())
		{
			if (m_pPowerGenerator[0] != nullptr)
			{
				for (int i = 0; m_pPowerGenerator[i]; i++)
				{
					u->attack(m_pPowerGenerator[i]);
				}
			}
			else if (m_nVultureCounter >= 4)
			{
				Unit enemySCV = u->getClosestUnit(GetType == UnitTypes::Terran_SCV && !IsOwned);//u->getUnitsInRadius(15, Filter::IsWorker && Filter::GetPlayer != Broodwar->self());
				if (enemySCV)
				{
					u->attack(enemySCV);
				}
				else if (m_nEnemyCommandCenterCounter == 0)
				{
					u->attack(_randomPosition);
				}
				else
					u->attack(Position(_enemyTilePosition));
			}
		}
		// If a vulture, a tank or factory has been hit, repair it
		if (u->getType() == UnitTypes::Terran_Vulture || u->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode || u->getType() == UnitTypes::Terran_Siege_Tank_Siege_Mode || u->getType() == UnitTypes::Terran_Factory && u->getPlayer() == Broodwar->self() && u->getHitPoints() < u->getType().maxHitPoints())
		{
			if (m_pSCVs[1] != nullptr && m_pSCVs[1]->isIdle())
			{
				m_pSCVs[1]->repair(u);
			}
		}

		// If it's a tank in tank mode, make it in siege mode
		if (u->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode && u->getPlayer() == Broodwar->self())
		{
			u->siege();
		}
	}
}

void BigAI::onUnitCreate(Unit u)
{
	/*
	** Resources
	*/
	// Gas
	if (u->getType() == UnitTypes::Resource_Vespene_Geyser)
		addUnit(m_pGas, u);
	// Mineral
	if (IsMineralField(u) && !u->isBeingGathered() && u->isVisible())
		addUnit(m_pMineral, u);
	// Power Generator
	if (u->getType() == UnitTypes::Special_Power_Generator && u->getPlayer() != Broodwar->self())
	{
		addUnit(m_pPowerGenerator, u);
		m_nPowerGenerator++;
	}
}

void BigAI::onUnitDestroy(Unit u)
{
	/*
	** Units
	*/
	// SCV
	if (u->getType() == UnitTypes::Terran_SCV && u->getPlayer() == Broodwar->self())
	{
		deleteUnit(m_pSCVs, u);
		m_nSCVCounter--;
	}
	// Marine
	if (u->getType() == UnitTypes::Terran_Marine && u->getPlayer() == Broodwar->self())
	{
		deleteUnit(m_pMarine, u);
		m_nMarineCounter--;
	}
	// Vulture
	if (u->getType() == UnitTypes::Terran_Vulture && u->getPlayer() == Broodwar->self())
	{
		deleteUnit(m_pVulture, u);
		m_nVultureCounter--;
	}
	// Tank
	if (u->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode && u->getPlayer() == Broodwar->self())
	{
		deleteUnit(m_pTank, u);
		m_nTankCounter--;
	}

	/*
	** Resources
	*/
	// Mineral
	if (IsMineralField(u) && u->isBeingGathered() && !u->isVisible())
		deleteUnit(m_pMineral, u);
	// Gas
	if (u->getType() == UnitTypes::Resource_Vespene_Geyser)
		deleteUnit(m_pGas, u);

	/*
	** Buildings
	*/
	// Refinery
	if (u->getType() == UnitTypes::Terran_Refinery && u->getPlayer() == Broodwar->self())
	{
		deleteUnit(m_pRefinery, u);
		m_nRefineryCounter--;
	}
	// Barrack
	if (u->getType() == UnitTypes::Terran_Barracks && u->getPlayer() == Broodwar->self())
	{
		deleteUnit(m_pBarracks, u);
		m_nBarrackCounter--;
	}
	// Academy
	if (u->getType() == UnitTypes::Terran_Academy && u->getPlayer() == Broodwar->self())
	{
		deleteUnit(m_pAcademy, u);
		m_nAcademyCounter--;
	}
	// Engineering bay
	if (u->getType() == UnitTypes::Terran_Engineering_Bay && u->getPlayer() == Broodwar->self())
	{
		deleteUnit(m_pEngineeringBay, u);
		m_nEngineeringBayCounter--;
	}
	// Armory
	if (u->getType() == UnitTypes::Terran_Armory && u->getPlayer() == Broodwar->self())
	{
		deleteUnit(m_pArmory, u);
		m_nArmoryCounter--;
	}
	// Bunker
	if (u->getType() == UnitTypes::Terran_Bunker && u->getPlayer() == Broodwar->self())
	{
		deleteUnit(m_pBunker, u);
		m_nBunkerCounter--;
	}
	// Factory
	if (u->getType() == UnitTypes::Terran_Factory && u->getPlayer() == Broodwar->self())
	{
		deleteUnit(m_pFactory, u);
		m_nFactoryCounter--;
	}
	// Missile turret
	if (u->getType() == UnitTypes::Terran_Missile_Turret && u->getPlayer() == Broodwar->self())
	{
		deleteUnit(m_pMissile_Turret, u);
		m_nMissileTurretCounter--;
	}
	// Refinery
	if (u->getType() == UnitTypes::Terran_Refinery && u->getPlayer() == Broodwar->self())
	{
		deleteUnit(m_pRefinery, u);
		m_nRefineryCounter--;
	}
	// Science Facility
	if (u->getType() == UnitTypes::Terran_Science_Facility && u->getPlayer() == Broodwar->self())
	{
		deleteUnit(m_pScienceFacility, u);
		m_nScienceFacilityCounter--;
	}
	// Starport
	if (u->getType() == UnitTypes::Terran_Starport && u->getPlayer() == Broodwar->self())
	{
		deleteUnit(m_pStarport, u);
		m_nStarportCounter--;
	}
	// Supply depot
	if (u->getType() == UnitTypes::Terran_Supply_Depot && u->getPlayer() == Broodwar->self())
	{
		deleteUnit(m_pSupplyDepot, u);
		m_nSupplyCounter--;
	}

	// Command Center
	if (u->getType() == UnitTypes::Terran_Command_Center && u->getPlayer() != Broodwar->self())
	{
		m_nEnemyCommandCenterCounter--;
	}

	// Power Generator
	if (u->getType() == UnitTypes::Special_Power_Generator && u->getPlayer() != Broodwar->self())
	{
		deleteUnit(m_pPowerGenerator, u);
		m_nPowerGenerator--;
	}

	// Machine Shop
	if (u->getType() == UnitTypes::Terran_Machine_Shop && u->getPlayer() == Broodwar->self())
	{
		deleteUnit(m_pMachineShop, u);
		m_nMachineShopCounter--;
	}
}

void BigAI::onUnitComplete(Unit u)
{
	/*
	** Units
	*/
	// SCV
	if (u->getType() == UnitTypes::Terran_SCV && u->getPlayer() == Broodwar->self())
	{
		addUnit(m_pSCVs, u);
		m_nSCVCounter++;
	}
	// Marine
	if (u->getType() == UnitTypes::Terran_Marine && u->getPlayer() == Broodwar->self())
	{
		addUnit(m_pMarine, u);
		m_nMarineCounter++;
	}
	// Vulture
	if (u->getType() == UnitTypes::Terran_Vulture && u->getPlayer() == Broodwar->self())
	{
		addUnit(m_pVulture, u);
		m_nVultureCounter++;
	}

	// Tank
	if (u->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode && u->getPlayer() == Broodwar->self())
	{
		addUnit(m_pTank, u);
		m_nTankCounter++;
	}

	/*
	** Buildings
	*/
	// Command Center
	if (u->getType() == UnitTypes::Terran_Command_Center && u->getPlayer() == Broodwar->self())
		m_CommandCenter = u;
	// Refinery
	if (u->getType() == UnitTypes::Terran_Refinery && u->getPlayer() == Broodwar->self())
	{
		addUnit(m_pRefinery, u);
		m_nRefineryCounter++;
	}
	// Barrack
	if (u->getType() == UnitTypes::Terran_Barracks && u->getPlayer() == Broodwar->self())
	{
		addUnit(m_pBarracks, u);
		m_nBarrackCounter++;
	}
	// Academy
	if (u->getType() == UnitTypes::Terran_Academy && u->getPlayer() == Broodwar->self())
	{
		addUnit(m_pAcademy, u);
		m_nAcademyCounter++;
	}
	// Engineering bay
	if (u->getType() == UnitTypes::Terran_Engineering_Bay && u->getPlayer() == Broodwar->self())
	{
		addUnit(m_pEngineeringBay, u);
		m_nEngineeringBayCounter++;
	}
	// Bunker
	if (u->getType() == UnitTypes::Terran_Bunker && u->getPlayer() == Broodwar->self())
	{
		addUnit(m_pBunker, u);
		m_nBunkerCounter++;
	}
	// Missile turret
	if (u->getType() == UnitTypes::Terran_Missile_Turret && u->getPlayer() == Broodwar->self())
	{
		addUnit(m_pMissile_Turret, u);
		m_nMissileTurretCounter++;
	}
	// Supply depot
	if (u->getType() == UnitTypes::Terran_Supply_Depot && u->getPlayer() == Broodwar->self())
	{
		addUnit(m_pSupplyDepot, u);
		m_nSupplyCounter++;
	}
	// Armory
	if (u->getType() == UnitTypes::Terran_Armory && u->getPlayer() == Broodwar->self())
	{
		addUnit(m_pArmory, u);
		m_nArmoryCounter++;
	}
	// Factory
	if (u->getType() == UnitTypes::Terran_Factory && u->getPlayer() == Broodwar->self())
	{
		addUnit(m_pFactory, u);
		m_nFactoryCounter++;
	}
	// Refinery
	if (u->getType() == UnitTypes::Terran_Refinery && u->getPlayer() == Broodwar->self())
	{
		addUnit(m_pRefinery, u);
		m_nRefineryCounter++;
	}
	// Science Facility
	if (u->getType() == UnitTypes::Terran_Science_Facility && u->getPlayer() == Broodwar->self())
	{
		addUnit(m_pScienceFacility, u);
		m_nScienceFacilityCounter++;
	}
	// Starport
	if (u->getType() == UnitTypes::Terran_Starport && u->getPlayer() == Broodwar->self())
	{
		addUnit(m_pStarport, u);
		m_nStarportCounter++;
	}
	// Machine Shop
	if (u->getType() == UnitTypes::Terran_Machine_Shop && u->getPlayer() == Broodwar->self())
	{
		addUnit(m_pMachineShop, u);
		m_nMachineShopCounter++;
	}

	if (u->getType() == UnitTypes::Terran_Barracks && u->getPlayer() == Broodwar->self())
		if (m_buildTree->GetCurrentNode() == BUILDBARRACK && m_bcheckBuild == true)
			m_bcheckBuild = false;

	if (u->getType() == UnitTypes::Terran_Factory && u->getPlayer() == Broodwar->self())
		if (m_buildTree->GetCurrentNode() == BUILDFACTORY && m_bcheckBuild == true)
			m_bcheckBuild = false;
}