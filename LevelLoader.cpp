#include "stdafx.h"
#include "LevelLoader.h"
#include "BinaryReader.h"
#include "GameObject.h"
#include "CubePrefab.h"
#include "PhysxManager.h"
#include "ContentManager.h"

#include "Components.h"
#include "../GameComponents/GameComponents.h"
#include "../../Materials/ColorMaterial.h"
#include "../../Materials/UberMaterial.h"

using namespace DirectX;

LevelData* LevelLoader::LoadContent(const std::wstring& assetFile) 
{
	BinaryReader* bReader(new BinaryReader());
	bReader->Open(assetFile);

	LevelData* pLevelData = new LevelData{};

	//Reading asset building prefabs
	std::wstring unparsedLine = bReader->ReadNullString();
	//Readlines until buffer character is encountered
	while (unparsedLine.length() && unparsedLine.at(0) != L'B')
	{
		//Filepaths and collision scale
		std::wstring pathMode = ReadString(unparsedLine);
		DirectX::XMFLOAT3 triggerScale = ReadFloat3(unparsedLine);
		std::wstring pathDiff = ReadString(unparsedLine);
		std::wstring pathSpec = ReadString(unparsedLine);
		std::wstring pathNorm = ReadString(unparsedLine);

		//Overal object: pos, rot and scale
		DirectX::XMFLOAT3 pos = ReadFloat3(unparsedLine);
		DirectX::XMFLOAT3 rot = ReadFloat3(unparsedLine);
		DirectX::XMFLOAT3 scale = ReadFloat3(unparsedLine);

		//Add the prefab to the level data
		pLevelData->AddStructureAsset(new BuildingAsset{ triggerScale, pos, rot, scale, pathMode, pathDiff, pathSpec, pathNorm });

		//Read next line
		unparsedLine = bReader->ReadNullString();
	}

	//Reading buildings
	unparsedLine = bReader->ReadNullString();
	//Readlines until buffer character is encountered
	while (unparsedLine.length() && unparsedLine.at(0) != L'B')
	{
		//Reading actual buildings, this will use the building prefabs (based on ID)
		//ID
		int id = (int)ReadInt(unparsedLine);
		//Position
		DirectX::XMFLOAT3 pos = ReadFloat3(unparsedLine);
		pos = DirectX::XMFLOAT3(pos.x * 1.5f, pos.y, pos.z * 3.f);
		//Rotation
		DirectX::XMFLOAT3 rot = ReadFloat3(unparsedLine);
		//Scale
		DirectX::XMFLOAT3 scale = ReadFloat3(unparsedLine);
		//health
		int health = (int)ReadInt(unparsedLine);

		//Add the to be created building to the level data
		BuildingData* pData = new BuildingData{ pos, rot, scale, id, health };
		pLevelData->AddBuilding(pData);

		//Read next line
		unparsedLine = bReader->ReadNullString();
	}

	//Reading railpositions
	unparsedLine = bReader->ReadNullString();
	//Readlines until buffer character is encountered
	while (unparsedLine.length() && unparsedLine.at(0) != L'B')
	{
		//Add a position to the railway
		pLevelData->AddCameraRailPos(ReadFloat3(unparsedLine));

		//Read next line
		unparsedLine = bReader->ReadNullString();
	}

	//Close and delete
	bReader->Close();
	SafeDelete(bReader);

	return pLevelData;
}

void LevelLoader::Destroy(LevelData* objToDestroy) 
{
	SafeDelete(objToDestroy);
}

std::wstring LevelLoader::ReadString(std::wstring& toRead) const
{
	//Find the end of the wstring as security
	//And grab the substring untill there
	size_t endPos = toRead.find(L";");
	std::wstring parsedLine = toRead.substr(0, endPos);

	//endpos + 1 as pos starts count from 0
	if (toRead.length() != endPos + 1) 
	{
		//Read from one after the to be removed part
		//For the length of the string - the end pos of the the removed part
		//Minus 1 as the endPos starts at 0 instead of 1
		int lengthRemainder = toRead.length() - endPos - 1;
		toRead = toRead.substr(endPos + 1, lengthRemainder);
	}
	else 
	{
		toRead.clear();
	}

	return parsedLine;
}

DirectX::XMFLOAT3 LevelLoader::ReadFloat3(std::wstring& toRead) const
{
	//Position
	//Find the end of the wstring as security
	//And grab the substring untill there
	size_t endPos = toRead.find(L";");
	std::wstring parsedLine = toRead.substr(0, endPos);

	if (std::string::npos != endPos)
	{
		//Read from one after the to be removed part
		//For the length of the string - the end pos of the the removed part
		//Minus 1 as the endPos starts at 0 instead of 1
		int lengthRemainder = toRead.length() - endPos - 1;
		toRead = toRead.substr(endPos + 1, lengthRemainder);
	}
	else
	{
		toRead.clear();
	}

	//x,y,z
	float x{}, y{}, z{};

	//Grab the substring
	endPos = parsedLine.find(L",");
	std::wstring temp = parsedLine.substr(0, endPos);
	int lengthRemainder = parsedLine.length() - temp.length() - 1;
	parsedLine = parsedLine.substr(endPos + 1, lengthRemainder);
	x = std::stof(temp);

	//Grab the substring
	endPos = parsedLine.find(L",");
	temp = parsedLine.substr(0, endPos);
	lengthRemainder = parsedLine.length() - temp.length() - 1;
	parsedLine = parsedLine.substr(endPos + 1, lengthRemainder);
	y = std::stof(temp);

	//Grab the substring
	endPos = parsedLine.find(L",");
	temp = parsedLine.substr(0, endPos);
	lengthRemainder = parsedLine.length() - temp.length() - 1;
	parsedLine = parsedLine.substr(endPos + 1, lengthRemainder);
	z = std::stof(temp);

	return DirectX::XMFLOAT3(x, y, z);
}

float LevelLoader::ReadFloat(std::wstring& toRead) const
{
	//Find the end of the wstring as security
	//And grab the substring untill there
	size_t endPos = toRead.find(L";");
	std::wstring parsedLine = toRead.substr(0, endPos);

	if (toRead.length() != endPos + 1)
	{		
		//Read from one after the to be removed part
		//For the length of the string - the end pos of the the removed part
		//Minus 1 as the endPos starts at 0 instead of 1
		int lengthRemainder = toRead.length() - endPos - 1;
		toRead = toRead.substr(endPos + 1, lengthRemainder);
	}
	else 
	{
		toRead.clear();
	}

	return std::stof(parsedLine);
}

int LevelLoader::ReadInt(std::wstring& toRead) const
{
	//Find the end of the wstring as security
	//And grab the substring untill there
	size_t endPos = toRead.find(L";");
	std::wstring parsedLine = toRead.substr(0, endPos);

	if (toRead.length() != endPos + 1)
	{
		//Read from one after the to be removed part
		//For the length of the string - the end pos of the the removed part
		//Minus 1 as the endPos starts at 0 instead of 1
		int lengthRemainder = toRead.length() - endPos - 1;
		toRead = toRead.substr(endPos + 1, lengthRemainder);
	}
	else 
	{
		toRead.clear();
	}

	return std::stoi(parsedLine);
}
