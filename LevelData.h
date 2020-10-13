#pragma once
#include <vector>

class GameObject;

struct BuildingAsset
{
	std::wstring meshAssetPath{};
	DirectX::XMFLOAT3 triggerScale{};
	std::wstring diffAssetPath{};
	std::wstring specAssetPath{};
	std::wstring normAssetPath{};
	DirectX::XMFLOAT3 pos{};
	DirectX::XMFLOAT3 rot{};
	DirectX::XMFLOAT3 scale{};
};

struct BuildingData 
{
	int id{};
	DirectX::XMFLOAT3 pos{};
	DirectX::XMFLOAT3 rot{};
	DirectX::XMFLOAT3 scale{};
	int health{};
};

class LevelData final
{
public:
	LevelData() = default;
	~LevelData();

	LevelData(LevelData& other) = delete;
	LevelData(LevelData&& other) = delete;
	LevelData operator=(LevelData& other) = delete;
	LevelData& operator=(LevelData&& other) = delete;

	void AddStructureAsset(BuildingAsset* data);
	void AddBuilding(BuildingData* data);
	void AddCameraRailPos(DirectX::XMFLOAT3 data);

	inline std::vector<BuildingAsset*> GetStructureAssets() { return m_StructureAssets; }
	inline std::vector<BuildingData*> GetBuildings() { return m_Buildings; }
	inline std::vector<DirectX::XMFLOAT3> GetCameraRailPoints() { return m_CameraRailsPoints; }

private:
	std::vector<BuildingAsset*> m_StructureAssets;
	std::vector<BuildingData*> m_Buildings;
	std::vector<DirectX::XMFLOAT3> m_CameraRailsPoints;
};