#include "MyMesh.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader); 

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));
	
	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);  

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue,-fValue, fValue); //0
	vector3 point1( fValue,-fValue, fValue); //1
	vector3 point2( fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue,-fValue,-fValue); //4
	vector3 point5( fValue,-fValue,-fValue); //5
	vector3 point6( fValue, fValue,-fValue); //6
	vector3 point7(-fValue, fValue,-fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	float subDivAngle = (2 * PI) / a_nSubdivisions; // Radians unfortunately
	float tempVal = 0;

	vector3 center(0.0f, 0.0f, 0.0f);
	vector3 tip(0.0f, 0.0f, -a_fHeight);
	vector3 start(a_fRadius * cosf(tempVal), a_fRadius * sinf(tempVal), 0.0f);
	vector3 prev = start;

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		if (i == a_nSubdivisions - 1)
		{
			AddTri(prev, center, start);
			AddTri(start, tip, prev);
			break;
		}

		tempVal -= subDivAngle;

		vector3 cur(a_fRadius * cosf(tempVal), a_fRadius * sinf(tempVal), 0);

		AddTri(prev, center, cur);
		AddTri(cur, tip, prev);

		prev = cur;
	}
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	float subDivAngle = (2 * PI) / a_nSubdivisions; // Radians
	float tempVal = 0;

	vector3 center(0.0f, 0.0f, 0.0f);
	vector3 centerTop(0.0f, 0.0f, -a_fHeight);
	vector3 start(a_fRadius * cosf(tempVal), a_fRadius * sinf(tempVal), 0.0f);
	vector3 startTop(a_fRadius * cosf(tempVal), a_fRadius * sinf(tempVal), -a_fHeight);
	vector3 prev = start;
	vector3 prevTop = startTop;

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		if (i == a_nSubdivisions - 1)
		{
			AddTri(prev, center, start);
			AddQuad(prev, start, prevTop, startTop);
			AddTri(startTop, centerTop, prevTop);
			break;
		}

		tempVal -= subDivAngle;

		vector3 cur(a_fRadius * cosf(tempVal), a_fRadius * sinf(tempVal), 0);
		vector3 curTop(a_fRadius * cosf(tempVal), a_fRadius * sinf(tempVal), -a_fHeight);

		AddTri(prev, center, cur);
		AddTri(curTop, centerTop, prevTop);
		AddQuad(prev, cur, prevTop, curTop);

		prev = cur;
		prevTop = curTop;
	}
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	float subDivAngle = (2 * PI) / a_nSubdivisions; // Radians
	float tempVal = 0;

	vector3 start(a_fInnerRadius * cosf(tempVal), a_fInnerRadius * sinf(tempVal), 0.0f);
	vector3 startExtended(a_fOuterRadius * cosf(tempVal), a_fOuterRadius * sinf(tempVal), 0.0f);
	vector3 startTop(a_fInnerRadius * cosf(tempVal), a_fInnerRadius * sinf(tempVal), -a_fHeight);
	vector3 startExtendedTop(a_fOuterRadius * cosf(tempVal), a_fOuterRadius * sinf(tempVal), -a_fHeight);

	vector3 prev = start;
	vector3 prevExtended = startExtended;
	vector3 prevTop = startTop;
	vector3 prevExtendedTop = startExtendedTop;

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		if (i == a_nSubdivisions - 1)
		{
			AddQuad(prev, start, prevExtended, startExtended); // Bottom
			AddQuad(start, prev, startTop, prevTop); // Inside
			AddQuad(startTop, prevTop, startExtendedTop, prevExtendedTop); // Top
			AddQuad(prevExtended, startExtended, prevExtendedTop, startExtendedTop); // Outside
			break;
		}

		tempVal -= subDivAngle;

		vector3 cur(a_fInnerRadius * cosf(tempVal), a_fInnerRadius * sinf(tempVal), 0);
		vector3 curExtended(a_fOuterRadius * cosf(tempVal), a_fOuterRadius * sinf(tempVal), 0);
		vector3 curTop(a_fInnerRadius * cosf(tempVal), a_fInnerRadius * sinf(tempVal), -a_fHeight);
		vector3 curExtendedTop(a_fOuterRadius * cosf(tempVal), a_fOuterRadius * sinf(tempVal), -a_fHeight);

		AddQuad(prev, cur, prevExtended, curExtended); // Bottom
		AddQuad(cur, prev, curTop, prevTop); // Inside
		AddQuad(prevExtendedTop, curExtendedTop, prevTop, curTop); // Top
		AddQuad(prevExtended, curExtended, prevExtendedTop, curExtendedTop); // Outside

		prev = cur;
		prevExtended = curExtended;
		prevTop = curTop;
		prevExtendedTop = curExtendedTop;
	}
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	// Replace this with your code

	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions > 6)
		a_nSubdivisions = 6;

	Release();
	Init();

	// Replace this with your code
	float subDivAngle = (2 * PI) / a_nSubdivisions; // Radians unfortunately
	float tempVal = 0;

	vector3 centerBottom(0.0f, a_fRadius, 0.0f);
	vector3 centerTop(0.0f, -a_fRadius, 0.0f);
	std::vector<vector3> curCircle;
	std::vector<vector3> prevCircle;

	for (int i = 0; i < a_nSubdivisions; i++) 
	{
		tempVal = 0;
		vector3 start((a_fRadius * cosf(tempVal)) / abs(cosf((i + 1) * PI)), (a_fRadius * cosf(((i + 1) * PI)) / a_nSubdivisions), (a_fRadius * sinf(tempVal)) / abs(cosf((i + 1) * PI)));
		vector3 prev = start;

		curCircle.push_back(start);
		for (int k = 0; k < a_nSubdivisions - 1; k++) 
		{
			tempVal -= subDivAngle;
			vector3 cur((a_fRadius * cosf(tempVal)) / abs(cosf((i + 1) * PI)), (a_fRadius * cosf(((i + 1) * PI)) / a_nSubdivisions), (a_fRadius * sinf(tempVal)) / abs(cosf((i + 1) * PI)));
			curCircle.push_back(cur);
		}
		
		if (i == 0) // Top Triangles
			for (int j = 0; j < curCircle.size(); j++)
			{
				if (j == a_nSubdivisions - 1)
					AddTri(curCircle[j], centerTop, start);
				else
					AddTri(curCircle[j], centerTop, curCircle[j + 1]);
			}
		else if (i == curCircle.size() - 1) // Bottom Triangles
			for (int j = 0; j < curCircle.size(); j++)
			{
				if (j == curCircle.size() - 1)
					AddTri(curCircle[j], centerBottom, start);
				else
					AddTri(curCircle[j + 1], centerBottom, curCircle[j]);
			}
		else
			for (int j = 0; j < curCircle.size(); j++)
			{
				if (j == curCircle.size() - 1)
					AddQuad(curCircle[0], curCircle[j], prevCircle[0], prevCircle[j]);
				else
					AddQuad(curCircle[j + 1], curCircle[j], prevCircle[j + 1], prevCircle[j]);
			}

		if (a_nSubdivisions != 1 && curCircle.size() == a_nSubdivisions)
			for (int k = curCircle.size() - 1; k >= 0; k--)
			{
				prevCircle.push_back(curCircle[k]);
				curCircle.pop_back();
			}
	}

	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}










/*

for (int j = 0; j < a_nSubdivisions; j++)
	{
		//vector3 start((a_fRadius * cosf(tempVal)) / (a_nSubdivisions), a_fRadius * (cosf(tempVal) / PI), (a_fRadius * sinf(tempVal)) / (a_nSubdivisions));
		vector3 prev = start;
		vector3 startBottom((a_fRadius * cosf(tempVal)) / (a_nSubdivisions), -a_fRadius * sinf((PI / 2) - ((PI / 2) / a_nSubdivisions)), (a_fRadius * sinf(tempVal)) / (a_nSubdivisions));
		vector3 prevBottom = startBottom;

		
		for (int i = 0; i < a_nSubdivisions; i++)
		{


			tempVal -= subDivAngle;
			vector3 curTop((a_fRadius * cosf(tempVal)) / (a_nSubdivisions), a_fRadius * sinf((PI / 2) - ((PI / 2) / a_nSubdivisions)), (a_fRadius * sinf(tempVal)) / (a_nSubdivisions));
			vector3 curBottom((a_fRadius * cosf(tempVal)) / (a_nSubdivisions), a_fRadius * sinf((PI / 2) - ((PI / 2) / a_nSubdivisions)), (a_fRadius * sinf(tempVal)) / (a_nSubdivisions));
			AddTri(curTop, centerTop, prevTop);
			// Insert connecting loop here
			AddTri(prevBottom, centerBottom, curBottom);

			if (i == 0)
			{
				AddTri(start, centerTop, cur);

			}
			else 
			{
				AddQuad(cur, prev)
			}

			prevTop = curTop;
			prevBottom = curBottom;

		}
	}

	/*
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		if (i == a_nSubdivisions - 1)
		{
			AddTri(startTop, centerTop, prevTop);

			// Insert connecting loop here
			// Values to store first top 2 pts and last bottom 2 pts
			vector3 tempPrev1 = prevTop;
			vector3 tempPrev2 = startTop;
			
			for (int j = 0; j < a_nSubdivisions - 2; j++) {
				
				vector3 tempCur1((a_fRadius * cosf(tempVal)) / (a_nSubdivisions), a_fRadius * sinf((PI / 2) - ((PI / 2) / a_nSubdivisions)), (a_fRadius * sinf(tempVal)) / (a_nSubdivisions));
				vector3 tempCur2();
			}

			AddTri(prevBottom, centerBottom, startBottom);
			break;
		}
		tempVal -= subDivAngle;
		vector3 curTop((a_fRadius * cosf(tempVal)) / (a_nSubdivisions), a_fRadius * sinf((PI / 2) - ((PI / 2) / a_nSubdivisions)), (a_fRadius * sinf(tempVal)) / (a_nSubdivisions));
		vector3 curBottom((a_fRadius * cosf(tempVal)) / (a_nSubdivisions), a_fRadius * sinf((PI / 2) - ((PI / 2) / a_nSubdivisions)), (a_fRadius * sinf(tempVal)) / (a_nSubdivisions));
		AddTri(curTop, centerTop, prevTop);
		// Insert connecting loop here
		AddTri(prevBottom, centerBottom, curBottom);
		prevTop = curTop;
		prevBottom = curBottom;
	}*/