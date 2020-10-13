#include "stdafx.h"
#include "ShipCharacter.h"
#include "Components.h"
#include "Prefabs.h"
#include "GameScene.h"
#include "PhysxManager.h"
#include "PhysxProxy.h"
#include "Bullet.h"
#include "SceneManager.h"
#include "GameScene.h"

using namespace DirectX;

ShipCharacter::ShipCharacter(float radius, float height, float moveSpeed) 
	: m_Radius(radius),
	m_Height(height),
	m_MoveSpeed(moveSpeed),
	m_pModelObject(nullptr),
	m_pCamera(nullptr),
	m_pController(nullptr),
	m_TotalPitch(0),
	m_TotalYaw(0),
	m_RotationSpeed(90.f),
	m_MaxFlyVelocity(5.0f),
	m_TerminalVelocity(20),
	m_Gravity(9.81f),
	m_FlyAccelerationTime(0.3f),
	m_FlyAcceleration(0.f),
	m_FlyVelocity(0, 0, 0),
	m_Velocity(0, 0, 0),
	m_RailDirection(0, 0, 0),
	m_MaxRelativeLoc(-12, 12 , -1.2f, 5),
	m_RelativeLoc(0, 0),
	m_MovementViewMatrix(),
	m_MovementRotationMatrix(),
	m_MaxRotationSpeed(250.f),
	m_RotationAccelerationTime(0.3f),
	m_RotationAcceleration(0.f),
	m_MaxRotation(30.f),
	m_RotationVelocity(0.f, 0.f, 0.f),
	m_CurrentRotation(0.f, 0.f, 0.f),
	m_BaseRotation(90.f, 0.f, 180.f),
	m_pHitBoxChild(nullptr),
	m_IsBoost(false),
	m_BoostDuration(1.8f),
	m_BoostCooldown(5.f),
	m_BoostTimer(0.f),
	m_pSpriteChild(nullptr)
{
	DirectX::XMStoreFloat4x4(&m_MovementViewMatrix, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&m_MovementRotationMatrix, DirectX::XMMatrixIdentity());

	m_FlyAcceleration = m_MaxFlyVelocity / m_FlyAccelerationTime;
	m_RotationAcceleration = m_MaxRotationSpeed / m_RotationAccelerationTime;
}

void ShipCharacter::Initialize(const GameContext& gameContext)
{
    //Add a controller component
    //While giving it a physics material as well
	physx::PxMaterial* pMaterial = PhysxManager::GetInstance()->GetPhysics()->createMaterial(0.5f,0.5f,0.5f);
	m_pController = new ControllerComponent(pMaterial);
	this->AddComponent(m_pController);

    //Add a camera component
	FixedCamera* pNewCamera = new FixedCamera();
	this->AddChild(pNewCamera);

	//INPUT
	//*****
	InputAction action = InputAction(CharacterMovement::UP,InputTriggerState::Down, 'W', -1, XINPUT_GAMEPAD_DPAD_UP);
	gameContext.pInput->AddInputAction(action);
	action = InputAction(CharacterMovement::LEFT, InputTriggerState::Down, 'A', -1, XINPUT_GAMEPAD_DPAD_LEFT);
	gameContext.pInput->AddInputAction(action);
	action = InputAction(CharacterMovement::DOWN, InputTriggerState::Down, 'S', -1, XINPUT_GAMEPAD_DPAD_DOWN);
	gameContext.pInput->AddInputAction(action);
	action = InputAction(CharacterMovement::RIGHT, InputTriggerState::Down, 'D', -1, XINPUT_GAMEPAD_DPAD_RIGHT);
	gameContext.pInput->AddInputAction(action);
	action = InputAction(CharacterMovement::BOOST, InputTriggerState::Pressed, VK_LSHIFT, -1, XINPUT_GAMEPAD_B);
	gameContext.pInput->AddInputAction(action);

	//COLLISION
	//*********
	GameObject* temp = new GameObject();
	RigidBodyComponent* pRigidBodyComponent = new RigidBodyComponent();
	temp->AddComponent(pRigidBodyComponent);
	pRigidBodyComponent->SetKinematic(true);
	std::shared_ptr<physx::PxGeometry> pCubeGeometry{ new physx::PxBoxGeometry(1,1,1) };
	temp->AddComponent(new ColliderComponent(pCubeGeometry, *pMaterial));
	AddChild(temp);
	m_pHitBoxChild = temp;
	m_pHitBoxChild->SetTag(L"PlayerHitBox");

	//EXHAUST SPRITE
	//**************
	m_pSpriteChild = new GameObject();
	ParticleEmitterComponent* pParticleEmitter = new ParticleEmitterComponent(L"./Resources/Textures/Smoke.png", 300);
	pParticleEmitter->SetVelocity(DirectX::XMFLOAT3(0, 0.0f, -1.f));
	pParticleEmitter->SetMinSize(0.2f);
	pParticleEmitter->SetMaxSize(0.3f);
	pParticleEmitter->SetMinEnergy(0.2f);
	pParticleEmitter->SetMaxEnergy(0.35f);
	pParticleEmitter->SetMinSizeGrow(0.5f);
	pParticleEmitter->SetMaxSizeGrow(1.0f);
	pParticleEmitter->SetMinEmitterRange(0.05f);
	pParticleEmitter->SetMaxEmitterRange(0.1f);
	pParticleEmitter->SetColor(DirectX::XMFLOAT4(1.f, 1.f, 1.f, 0.6f));
	m_pSpriteChild->AddComponent(pParticleEmitter);
	AddChild(m_pSpriteChild);
}

void ShipCharacter::PostInitialize(const GameContext& gameContext)
{
	UNREFERENCED_PARAMETER(gameContext);

    //Set the way the camera will look
	m_pCamera = GetChild<FixedCamera>()->GetComponent<CameraComponent>();
	m_pCamera->SetActive();
	m_pCamera->GetTransform()->Translate(0.f,-4.f,-12.f);
	m_pCamera->GetTransform()->Rotate(0.f, 0.f, 0.f, true);
}

void ShipCharacter::Update(const GameContext& gameContext)
{
	float elapsedSec{ gameContext.pGameTime->GetElapsed() };

	//BOOST
	//*****
    //Manage boost states
	m_BoostTimer += gameContext.pGameTime->GetElapsed();
	if (!m_IsBoost && m_BoostTimer >= m_BoostCooldown)
	{
		if (gameContext.pInput->IsActionTriggered(CharacterMovement::BOOST))
		{
			m_IsBoost = true;
			m_BoostTimer = 0.f;
		}
	}
	else if(m_IsBoost)
	{
		if (m_BoostTimer >= m_BoostDuration)
		{
			m_IsBoost = false;
			m_BoostTimer = 0.f;
		}
	}

	//MOVEMENT
	//********
	//HANDLE INPUT
	DirectX::XMFLOAT3 move = DirectX::XMFLOAT3(0, 0, 0);
	move.y = (gameContext.pInput->IsActionTriggered(CharacterMovement::UP) ? 1.0f : 0.0f)
		- (gameContext.pInput->IsActionTriggered(CharacterMovement::DOWN) ? 1.0f : 0.0f);
	move.x = (gameContext.pInput->IsActionTriggered(CharacterMovement::RIGHT) ? 1.0f : 0.0f)
		- (gameContext.pInput->IsActionTriggered(CharacterMovement::LEFT) ? 1.0f : 0.0f);

#pragma region velocities
	if (move.y != 0)
	{
		if ((move.y < 0.f && m_RotationVelocity.x < 0.f) || (move.y > 0.f && m_RotationVelocity.x < 0.f))
		{
			m_RotationVelocity.x = 0.f;
		}

		//Accelerate rotation
		m_RotationVelocity.x += m_RotationAcceleration * elapsedSec;
		m_RotationVelocity.x = std::min(m_RotationVelocity.x, m_MaxRotationSpeed);

		//Accelerate movement
		m_FlyVelocity.y += m_FlyAcceleration * elapsedSec;
		m_FlyVelocity.y = std::min(m_FlyVelocity.y, m_MaxFlyVelocity);
	}
	else
	{
		//Decelerate rotation
		if (m_CurrentRotation.x > 0.001f)
		{
			if (m_RotationVelocity.x > 0.f)
			{
				m_RotationVelocity.x = 0.f;
			}

			m_RotationVelocity.x -= m_RotationAcceleration * elapsedSec;
			m_RotationVelocity.x = std::max(m_RotationVelocity.x, -m_MaxRotationSpeed);
		}
		else if (m_CurrentRotation.x < -0.001f)
		{
			if (m_RotationVelocity.x < 0.f)
			{
				m_RotationVelocity.x = 0.f;
			}

			m_RotationVelocity.x += m_RotationAcceleration * elapsedSec;
			m_RotationVelocity.x = std::min(m_RotationVelocity.x, m_MaxRotationSpeed);
		}

		//Decelerate movement
		if (m_FlyVelocity.x != 0.f)
		{
			m_FlyVelocity.y -= m_FlyAcceleration * elapsedSec;
			m_FlyVelocity.y = std::max(m_FlyVelocity.x, 0.f);
		}
	}
	if (move.x != 0)
	{
		if ((move.x < 0.f && m_RotationVelocity.y < 0.f) || (move.x > 0.f && m_RotationVelocity.y < 0.f))
		{
			m_RotationVelocity.y = 0.f;
		}

		//Accelerate rotation
		m_RotationVelocity.y += m_RotationAcceleration * elapsedSec;
		m_RotationVelocity.y = std::min(m_RotationVelocity.y, m_MaxRotationSpeed);

		//Accelerate movement
		m_FlyVelocity.x += m_FlyAcceleration * elapsedSec;
		m_FlyVelocity.x = std::min(m_FlyVelocity.x, m_MaxFlyVelocity);
	}
	else
	{
		//Deccelerate rotation
		if (m_CurrentRotation.y > 0.001f)
		{
			if (m_RotationVelocity.y > 0.f)
			{
				m_RotationVelocity.y = 0.f;
			}

			m_RotationVelocity.y -= m_RotationAcceleration * elapsedSec;
			m_RotationVelocity.y = std::max(m_RotationVelocity.y, -m_MaxRotationSpeed);
		}
		else if (m_CurrentRotation.y < -0.001f)
		{
			if (m_RotationVelocity.y < 0.f)
			{
				m_RotationVelocity.y = 0.f;
			}

			m_RotationVelocity.y += m_RotationAcceleration * elapsedSec;
			m_RotationVelocity.y = std::min(m_RotationVelocity.y, m_MaxRotationSpeed);
		}

		//Deccelerate movement
		if (m_FlyVelocity.x != 0.f)
		{
			m_FlyVelocity.x -= m_FlyAcceleration * elapsedSec;
			m_FlyVelocity.x = std::max(m_FlyVelocity.x, 0.f);
		}
	}
#pragma endregion velocities

	//Orientation
	DirectX::XMStoreFloat3(&move, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&move), DirectX::XMLoadFloat4x4(&m_MovementViewMatrix)));

	//Rotation
	if (move.x != 0 || move.y != 0)
	{
        //Based on the current movement change the rotation over time
		m_CurrentRotation.x += -move.y * m_RotationVelocity.x * elapsedSec;
		if (std::abs(m_CurrentRotation.x) > m_MaxRotation)
			m_CurrentRotation.x = -move.y * m_MaxRotation;
		m_CurrentRotation.y += move.x * m_RotationVelocity.y * elapsedSec;
		if (std::abs(m_CurrentRotation.y) > m_MaxRotation)
			m_CurrentRotation.y = move.x * m_MaxRotation;
	}
	if (move.y == 0)
	{
        //No movement, return to normal
		m_CurrentRotation.x += m_RotationVelocity.x * elapsedSec;
	}
	if (move.x == 0)
	{
        //No movement, return to normal
		m_CurrentRotation.y += m_RotationVelocity.y * elapsedSec;
	}
    //Add the calculated change to the base rotation
	m_pModelObject->GetTransform()->Rotate(
		m_BaseRotation.x + m_CurrentRotation.x,
		m_BaseRotation.y + m_CurrentRotation.y,
		m_BaseRotation.z + m_CurrentRotation.z);
	DirectX::XMFLOAT3 position = m_pModelObject->GetTransform()->GetPosition();
    //Change position of the sprites to line up with the exhausts
	m_pSpriteChild->GetTransform()->Translate(position.x + tanf(m_CurrentRotation.y) * 0.05f, position.y - 0.3f - tanf(m_CurrentRotation.x) * 0.05f, position.z - 0.5f);

	//Movement
	m_Velocity = DirectX::XMFLOAT3(m_FlyVelocity.x * move.x, m_FlyVelocity.y * move.y, 0.f);
	Move(DirectX::XMFLOAT3(m_Velocity.x * elapsedSec, m_Velocity.y * elapsedSec, 0.f));
}

void ShipCharacter::Move(const DirectX::XMFLOAT3 displacement)
{
	//Limit movement
	DirectX::XMFLOAT3 clampedDisplacement = displacement;
	if (m_MaxRelativeLoc.x > m_RelativeLoc.x && clampedDisplacement.x < 0.f)
		clampedDisplacement.x = 0.f;
	if (m_MaxRelativeLoc.y < m_RelativeLoc.x && clampedDisplacement.x > 0.f)
		clampedDisplacement.x = 0.f;
	if (m_MaxRelativeLoc.z > m_RelativeLoc.y && clampedDisplacement.y < 0.f)
		clampedDisplacement.y = 0.f;
	if (m_MaxRelativeLoc.w < m_RelativeLoc.y && clampedDisplacement.y > 0.f)
		clampedDisplacement.y = 0.f;
	m_RelativeLoc.x += clampedDisplacement.x;
	m_RelativeLoc.y += clampedDisplacement.y;

	//Controller
	m_pController->Move(clampedDisplacement, 0.f);
	//Model
	DirectX::XMFLOAT3 temp;
	const DirectX::XMFLOAT3 modelPos = m_pModelObject->GetTransform()->GetPosition();
	DirectX::XMStoreFloat3(&temp, DirectX::XMLoadFloat3(&modelPos) + DirectX::XMLoadFloat3(&clampedDisplacement));
	m_pModelObject->GetTransform()->Translate(temp);
	//Box
	m_pHitBoxChild->GetTransform()->Translate(temp.x, temp.y, temp.z);
}

void ShipCharacter::Translate(const DirectX::XMFLOAT3 destination)
{
	//Controller
	m_pController->Translate(destination);
	//Model
	m_pModelObject->GetTransform()->Translate(destination);
	//Sprites
	m_pSpriteChild->GetTransform()->Translate(destination);
}
