#ifndef PLAYSTATE_H
#define PLAYSTATE_H

//#include "SDL2/SDL.h"
#include "gamestate.h"
#include "rcubeobject.h"
#include "shapes.h"

enum GameKeys
{
    GK_LOOKUP, GK_LOOKDOWN, GK_LOOKLEFT, GK_LOOKRIGHT, GK_ROTATECLOCKWISE, GK_ROTATECOUNTERCLOCKWISE,

    GK_MOVEFRONT, GK_MOVEFRONTINV, GK_MOVEBACK, GK_MOVEBACKINV,
    GK_MOVELEFT, GK_MOVELEFTINV, GK_MOVERIGHT, GK_MOVERIGHTINV,
    GK_MOVEUP, GK_MOVEUPINV, GK_MOVEDOWN, GK_MOVEDOWNINV,
    GK_MOVEFIRST = GK_MOVEFRONT,
    GK_MOVELAST = GK_MOVEDOWNINV,

    GK_MOVEMOUSE,

    GK_CHANGECOLOR,
    GK_CHANGEPROJ,
    GK_COUNT
};

enum ProjectionType
{
	PT_DIMETRIC, PT_ISOMETRIC,
	PT_COUNT
};

class GameKeysStates
{
public:
	inline void setDown() {
		if ( !m_isPressed )
		{
			m_isNewDown = true;
			m_isPressed = true;
		}
	};
	inline void releasePress() { if ( m_isPressed ) m_isPressed = false; };
	inline void releaseNewDown() { if ( m_isNewDown ) m_isNewDown = false; };
	inline bool isNewDown() const { return m_isNewDown; };

private:
    bool m_isNewDown = { 0 };
    bool m_isPressed = { 0 };
};

class CPlayState : public CGameState
{
public:
	void Init();
	void Cleanup();

	void Pause();
	void Resume();

	void HandleEvents( CGameEngine * game );
	void Update( CGameEngine * game );
	void Draw( CGameEngine * game );

	static CPlayState * Instance() { return &m_PlayState; }

protected:
	CPlayState() { }

private:
	static CPlayState m_PlayState;
	RCubeObject * m_RCube;
	bool m_needRedraw = { true };
	GameKeysStates m_gkStates[ GK_COUNT ];
	ProjectionType m_prType = { PT_ISOMETRIC };
	Point3D m_pBegin, m_pEnd;

	void setProjection( const ProjectionType pType ) const;
	Point3D getGLPos( const int mX, const int mY ) const;

	ShaderProgram * m_shaderPr;
	GLuint vbo_cube_vertices, vbo_cube_texcoords;
	GLuint ibo_cube_elements;
	GLuint m_textureRedID, m_textureYellowID;
	GLint attribute_coord3d, attribute_texcoord;
	GLint uniform_mvp, m_unTextureRed, m_unTextureYellow;
	GLfloat cube_texcoords[2*10] = {
		// front
		0.0, 0.0,	1.0, 0.0,	1.0, 1.0,	0.0, 1.0,
		1.0, 0.0,	0.0, 0.0,	1.0, 1.0,	0.0, 1.0,
		1.0, 1.0,   0.0, 0.0,
	};

/*	GLfloat cube_vertices[3*4*6] = {
		// front
		-1.0, -1.0,  1.0,
		1.0, -1.0,  1.0,
		1.0,  1.0,  1.0,
		-1.0,  1.0,  1.0,
		// top
		-1.0,  1.0,  1.0,
		1.0,  1.0,  1.0,
		1.0,  1.0, -1.0,
		-1.0,  1.0, -1.0,
		// back
		1.0, -1.0, -1.0,
		-1.0, -1.0, -1.0,
		-1.0,  1.0, -1.0,
		1.0,  1.0, -1.0,
		// bottom
		-1.0, -1.0, -1.0,
		1.0, -1.0, -1.0,
		1.0, -1.0,  1.0,
		-1.0, -1.0,  1.0,
		// left
		-1.0, -1.0, -1.0,
		-1.0, -1.0,  1.0,
		-1.0,  1.0,  1.0,
		-1.0,  1.0, -1.0,
		// right
		1.0, -1.0,  1.0,
		1.0, -1.0, -1.0,
		1.0,  1.0, -1.0,
		1.0,  1.0,  1.0,
	};
*/

	GLfloat cube_vertices[ 3*10 ] = {
		// front
		-1.0, -1.0,  1.0,
		 1.0, -1.0,  1.0,
		 1.0,  1.0,  1.0,
		-1.0,  1.0,  1.0,
		 1.0,  1.0, -1.0,
		-1.0,  1.0, -1.0,
		 1.0, -1.0, -1.0,
		-1.0, -1.0, -1.0,
		-1.0,  1.0, -1.0,
		 1.0, -1.0, -1.0,
	};

	GLushort cube_elements[ 4*6 ] = {
		0, 1, 2, 3, // front
		2, 4, 5, 3, // top
		6, 7, 5, 4, // back
		6, 1, 0, 7, // bottom
		0, 3, 8, 7, // left
		2, 1, 9, 4, // right
	};
/*	GLushort cube_elements[2*3*6] = {
		// front
		0,  1,  2,
		2,  3,  0,
		// top
		4,  5,  6,
		6,  7,  4,
		// back
		8,  9, 10,
		10, 11,  8,
		// bottom
		12, 13, 14,
		14, 15, 12,
		// left
		16, 17, 18,
		18, 19, 16,
		// right
		20, 21, 22,
		22, 23, 20,
	};
*/
};

#endif
