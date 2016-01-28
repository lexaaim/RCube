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
	GLint attribute_coord2d, attribute_v_color;
	GLint uniform_fade;
	GLuint vbo_triangle;

	struct attributes {
		GLfloat coord2d[2];
		GLfloat v_color[3];
	};
	struct attributes triangle_attributes[3] = {
		{{ 0.0,  0.8}, {1.0, 1.0, 0.0}},
		{{-0.8, -0.8}, {0.0, 0.0, 1.0}},
		{{ 0.8, -0.8}, {1.0, 0.0, 0.0}}
	};
};

#endif
