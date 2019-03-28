#include "MyOctant.h"
using namespace Simplex;

uint Simplex::MyOctant::m_uOctantCount = 0;
uint Simplex::MyOctant::m_uMaxLevel = 3;
uint Simplex::MyOctant::m_uIdealEntityCount = 5;

Simplex::MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	Init();

	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCount;
	m_pRoot = this;
	m_uLevel = 0;

	uint entityCount = m_pEntityMngr->GetEntityCount();
	
	// Determine the minimum and maximum coords of this octant
	// Add all entities to the entity list
	for (uint i = 0; i < entityCount; ++i) 
	{
		vector3 entityMax = vector3(m_pEntityMngr->GetEntity(i)->GetRigidBody()->GetMaxGlobal());
		vector3 entityMin = vector3(m_pEntityMngr->GetEntity(i)->GetRigidBody()->GetMinGlobal());

		m_v3Min.x = (entityMin.x < m_v3Min.x) ? entityMin.x : m_v3Min.x;
		m_v3Min.y = (entityMin.y < m_v3Min.y) ? entityMin.y : m_v3Min.y;
		m_v3Min.z = (entityMin.z < m_v3Min.z) ? entityMin.z : m_v3Min.z;

		m_v3Max.x = (entityMax.x > m_v3Max.x) ? entityMax.x : m_v3Max.x;
		m_v3Max.y = (entityMax.y > m_v3Max.y) ? entityMax.y : m_v3Max.y;
		m_v3Max.z = (entityMax.z > m_v3Max.z) ? entityMax.z : m_v3Max.z;

		m_EntityList.push_back(i);
	}

	// Determine the center of this octant
	m_v3Center = (m_v3Min + m_v3Max) / 2.0f;

	// Set the size to that of the largest side
	m_fSize = m_v3Max.x - m_v3Min.x;
	m_fSize = (m_v3Max.y - m_v3Min.y) > m_fSize ? (m_v3Max.y - m_v3Min.y) : m_fSize;
	m_fSize = (m_v3Max.z - m_v3Min.z) > m_fSize ? (m_v3Max.z - m_v3Min.z) : m_fSize;

	m_v3Min = m_v3Center - vector3(m_fSize / 2.0f);
	m_v3Max = m_v3Center + vector3(m_fSize / 2.0f);

	// Subdivide the tree based on the given max level and ideal entity count
	ConstructTree(m_uMaxLevel);

	// Get all leaf nodes that have entities
	ConstructList();

	// Add dimensions based on the current octants
	AssignIDtoEntity();

}

Simplex::MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
{
	Init();
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;
	m_v3Min = vector3(m_v3Center) - vector3(m_fSize / 2.0f);
	m_v3Max = vector3(m_v3Center) + vector3(m_fSize / 2.0f);
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
	for (int i = 0; i < (sizeof(m_pChild) / sizeof(*m_pChild)); ++i) 
	{
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
	MyRigidBody* entityBody = m_pEntityMngr->GetEntity(a_uRBIndex)->GetRigidBody();

	if (this->m_v3Max.x < entityBody->GetMinGlobal().x) //this to the right of other
		return false;
	if (this->m_v3Min.x > entityBody->GetMaxGlobal().x) //this to the left of other
		return false;

	if (this->m_v3Max.y < entityBody->GetMinGlobal().y) //this below of other
		return false;
	if (this->m_v3Min.y > entityBody->GetMaxGlobal().y) //this above of other
		return false;

	if (this->m_v3Max.z < entityBody->GetMinGlobal().z) //this behind of other
		return false;
	if (this->m_v3Min.z > entityBody->GetMaxGlobal().z) //this in front of other
		return false;

	return true;
}

bool Simplex::MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
{
	if (m_uID == a_nIndex)
	{
		Display(a_v3Color);
		return true;
	}
	for (uint i = 0; i < m_uChildren; ++i) {
		if (m_pChild[i]->Display(a_nIndex, a_v3Color))
		{
			return true;
		}
	}
	return false;
}

void Simplex::MyOctant::Display(vector3 a_v3Color)
{
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color);
}

void Simplex::MyOctant::DisplayLeafs(vector3 a_v3Color)
{
	if (m_EntityList.size() < 1) 
	{
		return;
	}
	if (IsLeaf()) 
	{
		this->Display(a_v3Color);
		return;
	}
	for (uint i = 0; i < m_uChildren; ++i) 
	{
		m_pChild[i]->DisplayLeafs();
	}
}

void Simplex::MyOctant::ClearEntityList(void)
{
	m_EntityList.clear();
	for (uint i = 0; i < m_uChildren; ++i) {
		m_pChild[i]->ClearEntityList();
	}
}

void Simplex::MyOctant::Subdivide(void)
{
	vector3 corner[8];
	corner[0] = m_v3Min;
	corner[1] = vector3(m_v3Max.x, m_v3Min.y, m_v3Min.z);
	corner[2] = vector3(m_v3Max.x, m_v3Min.y, m_v3Max.z);
	corner[3] = vector3(m_v3Min.x, m_v3Min.y, m_v3Max.z);
	corner[4] = vector3(m_v3Min.x, m_v3Max.y, m_v3Max.z);
	corner[5] = vector3(m_v3Min.x, m_v3Max.y, m_v3Min.z);
	corner[6] = vector3(m_v3Max.x, m_v3Max.y, m_v3Min.z);
	corner[7] = m_v3Max;
	m_uChildren = 8;
	for (uint i = 0; i < m_uChildren; ++i) 
	{
		m_pChild[i] = new MyOctant((corner[i] + m_v3Center) / 2, m_fSize / 2);
		m_pChild[i]->m_pParent = this;
		m_pChild[i]->m_uLevel = this->m_uLevel + 1;
		m_pChild[i]->m_uChildren = 0;
		m_pChild[i]->m_pRoot = this->m_pRoot;

		uint totalEntities = m_EntityList.size();
		for (uint j = 0; j < totalEntities; ++j)
		{
			if (m_pChild[i]->IsColliding(m_EntityList[j]))
			{
				m_pChild[i]->m_EntityList.push_back(m_EntityList[j]);
			}
		}
	}
}

MyOctant * Simplex::MyOctant::GetChild(uint a_nChild)
{
	if (a_nChild < m_uChildren) 
	{
		return m_pChild[a_nChild];
	}
	return nullptr;
}

MyOctant * Simplex::MyOctant::GetParent(void)
{
	return m_pParent;
}

bool Simplex::MyOctant::IsLeaf(void)
{
	return m_uChildren < 1;
}

bool Simplex::MyOctant::ContainsMoreThan(uint a_nEntities)
{
	return m_EntityList.size() > a_nEntities;
}

void Simplex::MyOctant::KillBranches(void)
{
	for (uint i = 0; i < m_uChildren; ++i)
	{
		SafeDelete(m_pChild[i]);
		m_pChild[i] = nullptr;
	}
	m_uChildren = 0;
}

void Simplex::MyOctant::ConstructTree(uint a_nMaxLevel)
{
	if (m_uLevel >= a_nMaxLevel)
	{
		return;
	}
	if (m_uIdealEntityCount >= m_EntityList.size())
	{
		return;
	}
	Subdivide();
	for (uint i = 0; i < m_uChildren; ++i)
	{
		m_pChild[i]->ConstructTree(a_nMaxLevel);
	}
}

void Simplex::MyOctant::AssignIDtoEntity(void)
{
	uint totalNonEmptyLeafs = m_lChild.size();
	for (int i = 0; i < totalNonEmptyLeafs; ++i)
	{
		uint entitiesInLeaf = m_lChild[i]->m_EntityList.size();
		for (int j = 0; j < entitiesInLeaf; ++j)
		{
			m_pEntityMngr->AddDimension(m_lChild[i]->m_EntityList[j], m_lChild[i]->m_uID);
		}

	}
}

uint Simplex::MyOctant::GetOctantCount(void)
{
	return m_uOctantCount;
}

void Simplex::MyOctant::Release(void)
{
	m_pMeshMngr = nullptr;
	m_pEntityMngr = nullptr;
	m_pParent = nullptr; 
	m_pRoot = nullptr;

	KillBranches();

	--m_uOctantCount;
}

void Simplex::MyOctant::Init(void)
{
	m_uID = m_uOctantCount;
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
	for (uint i = 0; i < 8; ++i)
	{
		m_pChild[i] = nullptr;
	}

	++m_uOctantCount;
}

void Simplex::MyOctant::ConstructList(void)
{
	if (IsLeaf())
	{
		if (ContainsMoreThan(0))
		{
			m_pRoot->m_lChild.push_back(this);
		}
		return;
	}
	for (uint i = 0; i < m_uChildren; ++i)
	{
		m_pChild[i]->ConstructList();
	}
}
