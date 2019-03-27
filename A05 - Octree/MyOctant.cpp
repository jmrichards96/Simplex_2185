#include "MyOctant.h"
using namespace Simplex;

uint Simplex::MyOctant::m_uOctantCount = 0;
uint Simplex::MyOctant::m_uMaxLevel = 0;
uint Simplex::MyOctant::m_uIdealEntityCount = 0;

Simplex::MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	Init();

	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCount;

	uint entityCount = m_pEntityMngr->GetEntityCount();
	
	// Determine the minimum and maximum coords of this octant
	for (int i = 0; i < entityCount; ++i) {
		vector3 entityPos = vector3(m_pEntityMngr->GetEntity(i)->GetModelMatrix() * vector4(ZERO_V3, 1.0f));

		m_v3Min.x = (entityPos.x < m_v3Min.x) ? entityPos.x : m_v3Min.x;
		m_v3Min.y = (entityPos.y < m_v3Min.y) ? entityPos.y : m_v3Min.y;
		m_v3Min.z = (entityPos.z < m_v3Min.z) ? entityPos.z : m_v3Min.z;

		m_v3Max.x = (entityPos.x > m_v3Max.x) ? entityPos.x : m_v3Max.x;
		m_v3Max.y = (entityPos.y > m_v3Max.y) ? entityPos.y : m_v3Max.y;
		m_v3Max.z = (entityPos.z > m_v3Max.z) ? entityPos.z : m_v3Max.z;
	}

	// Determine the center of this octant
	m_v3Center = (m_v3Min + m_v3Max) / 2.0f;

	// Set the size to that of the largest side
	m_fSize = m_v3Max.x - m_v3Min.x;
	m_fSize = (m_v3Max.y - m_v3Min.y) > m_fSize ? (m_v3Max.y - m_v3Min.y) : m_fSize;
	m_fSize = (m_v3Max.z - m_v3Min.z) > m_fSize ? (m_v3Max.z - m_v3Min.z) : m_fSize;

}

Simplex::MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
{
	Init();
	m_v3Center = m_v3Center;
	m_fSize = a_fSize;
}

Simplex::MyOctant::MyOctant(MyOctant const & other)
{
	m_uOctantCount = other.m_uOctantCount;
	m_uMaxLevel = other.m_uMaxLevel;
	m_uIdealEntityCount = other.m_uIdealEntityCount;

	m_uID = other.m_uID;
	m_uLevel = other.m_uLevel;
	m_uChildren = other.m_uChildren;

	m_fSize = other.m_fSize;

	m_pMeshMngr = other.m_pMeshMngr;
	m_pEntityMngr = other.m_pEntityMngr;

	m_v3Center = other.m_v3Center;
	m_v3Min = other.m_v3Min;
	m_v3Max = other.m_v3Max;

	m_pParent = other.m_pParent;
	for (int i = 0; i < (sizeof(m_pChild) / sizeof(*m_pChild)); ++i) {
		m_pChild[i] = other.m_pChild[i];
	}
	m_EntityList = other.m_EntityList;
	m_pRoot = other.m_pRoot;
	m_lChild = other.m_lChild;
}

MyOctant & Simplex::MyOctant::operator=(MyOctant const & other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyOctant temp(other);
		Swap(temp);
	}
	return *this;
}

Simplex::MyOctant::~MyOctant(void)
{
	Release();
}

void Simplex::MyOctant::Swap(MyOctant & other)
{
	std::swap(m_uOctantCount, other.m_uOctantCount);
	std::swap(m_uMaxLevel, other.m_uMaxLevel);
	std::swap(m_uIdealEntityCount, other.m_uIdealEntityCount);

	std::swap(m_uID, other.m_uID);
	std::swap(m_uLevel, other.m_uLevel);
	std::swap(m_uChildren, other.m_uChildren);

	std::swap(m_fSize, other.m_fSize);

	std::swap(m_pMeshMngr, other.m_pMeshMngr);
	std::swap(m_pEntityMngr, other.m_pEntityMngr);

	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_v3Max, other.m_v3Max);

	std::swap(m_pParent, other.m_pParent);
	std::swap(m_pChild, other.m_pChild);
	std::swap(m_EntityList, other.m_EntityList);
	std::swap(m_pRoot, other.m_pRoot);
	std::swap(m_lChild, other.m_lChild);
}

float Simplex::MyOctant::GetSize(void)
{
	return m_fSize;
}

vector3 Simplex::MyOctant::GetCenterGlobal(void)
{
	return m_v3Center;
}

vector3 Simplex::MyOctant::GetMinGlobal(void)
{
	return m_v3Min;
}

vector3 Simplex::MyOctant::GetMaxGlobal(void)
{
	return m_v3Max;
}

bool Simplex::MyOctant::IsColliding(uint a_uRBIndex)
{
	return false;
}

void Simplex::MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
{
}

void Simplex::MyOctant::Display(vector3 a_v3Color)
{
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color);
}

void Simplex::MyOctant::DisplayLeafs(vector3 a_v3Color)
{
	if (m_EntityList.size() < 1) {
		return;
	}
	if (IsLeaf()) {
		this->Display(a_v3Color);
		return;
	}
	for (int i; i < m_uChildren; ++i) {
		m_pChild[i]->DisplayLeafs();
	}
}

void Simplex::MyOctant::ClearEntityList(void)
{
	m_EntityList.clear();
	for (int i; i < m_uChildren; ++i) {
		m_pChild[i]->ClearEntityList();
	}
}

void Simplex::MyOctant::Subdivide(void)
{
}

MyOctant * Simplex::MyOctant::GetChild(uint a_nChild)
{
	return m_pChild[a_nChild];
}

MyOctant * Simplex::MyOctant::GetParent(void)
{
	return nullptr;
}

bool Simplex::MyOctant::IsLeaf(void)
{
	return m_uChildren < 1;
}

bool Simplex::MyOctant::ContainsMoreThan(uint a_nEntities)
{
	return false;
}

void Simplex::MyOctant::KillBranches(void)
{
}

void Simplex::MyOctant::ConstructTree(uint a_nMaxLevel)
{
}

void Simplex::MyOctant::AssignIDtoEntity(void)
{
}

uint Simplex::MyOctant::GetOctantCount(void)
{
	return uint();
}

void Simplex::MyOctant::Release(void)
{
	m_pMeshMngr = nullptr;
	m_pEntityMngr = nullptr;
	m_pParent = nullptr; 
	m_pRoot = nullptr;

	for (int i = 0; i < m_uChildren; ++i) {
		SafeDelete(m_pChild[i]);
	}
}

void Simplex::MyOctant::Init(void)
{

	m_uOctantCount = 0;
	m_uMaxLevel = 0;
	m_uIdealEntityCount = 0;

	m_uID = 0;
	m_uLevel = 0;
	m_uChildren = 0;

	m_fSize = 0.0f;

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	m_v3Center = vector3(0.0f);
	m_v3Min = vector3(0.0f);
	m_v3Max = vector3(0.0f);

	m_pParent = nullptr;

	m_EntityList.clear();

	m_pRoot = nullptr;
	m_lChild.clear();
}

void Simplex::MyOctant::ConstructList(void)
{
}
