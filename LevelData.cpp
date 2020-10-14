#include "stdafx.h"
#include "LevelData.h"
#include "GameObject.h"
#include "../GameComponents/HealthComponent.h"

LevelData::~LevelData()
{
	//Deleting the building prefabs
	for (size_t idx = 0; idx < m_StructureAssets.size(); idx++)
	{
		SafeDelete(m_StructureAssets[idx]);
	}
	m_StructureAssets.clear();

	//Deleting the building instances
	for (size_t idx = 0; idx < m_Buildings.size(); idx++)
	{
		SafeDelete(m_Buildings[idx]);
	}
	m_Buildings.clear();
}

void LevelData::AddStructureAsset(BuildingAsset* data)
{
	//Check if a duplicate of the asset could be found
	std::vector<BuildingAsset*>::iterator it = std::find(m_StructureAssets.begin(), m_StructureAssets.end(), data);
	if (it == m_StructureAssets.end()) 
	{
		//No duplicates found so push back to the vector
		m_StructureAssets.push_back(data);
	}
}

void LevelData::AddBuilding(BuildingData* data)
{
	//Check if a duplicate of the asset could be found
	std::vector<BuildingData*>::iterator it = std::find(m_Buildings.begin(), m_Buildings.end(), data);
	if (it == m_Buildings.end()) 
	{
		//No duplicates found so push back to the vector
		m_Buildings.push_back(data);
	}
}

void LevelData::AddCameraRailPos(DirectX::XMFLOAT3& data)
{
	//The rail does not check for duplicates
	//It can pass over the same spot multiple times
	m_CameraRailsPoints.push_back(data);
}
