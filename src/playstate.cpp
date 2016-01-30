#include "all.h"

#include "playstate.h"
#include "gamestate.h"
#include "gameengine.h"
#include "myquaternion.h"
#include "rcubeparams.h"

#include "shader.h"
#include "shaderprogram.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

CPlayState CPlayState::m_PlayState;

void CPlayState::Init()
{
	m_RCube = new RCubeObject;
/*
	for (int i = 1; i < 6; i++)
		memcpy(&cube_texcoords[i*4*2], &cube_texcoords[0], 2*4*sizeof(GLfloat));
*/
	glGenBuffers( 1, &vbo_cube_vertices );
	glBindBuffer( GL_ARRAY_BUFFER, vbo_cube_vertices );
	glBufferData( GL_ARRAY_BUFFER, sizeof( cube_vertices ), cube_vertices, GL_STATIC_DRAW );

	glGenBuffers( 1, &vbo_cube_texcoords );
	glBindBuffer( GL_ARRAY_BUFFER, vbo_cube_texcoords );
	glBufferData( GL_ARRAY_BUFFER, sizeof( cube_texcoords ), cube_texcoords, GL_STATIC_DRAW );

	glGenBuffers( 1, &ibo_cube_elements );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( cube_elements ), cube_elements, GL_STATIC_DRAW );

	SDL_Surface* res_texture = IMG_Load( "glsl/red.png" );
	if (res_texture == NULL) {
		std::cout << "IMG_Load: " << SDL_GetError() << std::endl;
		return;
	}
	glGenTextures(1, &m_textureRedID);
	glBindTexture(GL_TEXTURE_2D, m_textureRedID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, // target
		0,  // level, 0 = base, no minimap,
		GL_RGBA, // internalformat
		res_texture->w,  // width
		res_texture->h,  // height
		0,  // border, always 0 in OpenGL ES
		GL_RGBA,  // format
		GL_UNSIGNED_BYTE, // type
		res_texture->pixels);
	SDL_FreeSurface(res_texture);

	res_texture = IMG_Load( "glsl/yellow.png" );
	if (res_texture == NULL) {
		std::cout << "IMG_Load: " << SDL_GetError() << std::endl;
		return;
	}
	glGenTextures(1, &m_textureYellowID );
	glBindTexture(GL_TEXTURE_2D, m_textureYellowID );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, // target
		0,  // level, 0 = base, no minimap,
		GL_RGBA, // internalformat
		res_texture->w,  // width
		res_texture->h,  // height
		0,  // border, always 0 in OpenGL ES
		GL_RGBA,  // format
		GL_UNSIGNED_BYTE, // type
		res_texture->pixels);
	SDL_FreeSurface(res_texture);

	//
	Shader vertexShader( GL_VERTEX_SHADER );
	vertexShader.loadFromFile( "glsl/shader.v.glsl" );
	vertexShader.compile();

	// Set up fragment shader
	Shader fragmentShader( GL_FRAGMENT_SHADER );
	fragmentShader.loadFromFile( "glsl/shader.f.glsl" );
	fragmentShader.compile();

	// Set up shader program
	m_shaderPr = new ShaderProgram();
	m_shaderPr->attachShader( vertexShader );
	m_shaderPr->attachShader( fragmentShader );
	m_shaderPr->linkProgram();

	attribute_coord3d = m_shaderPr->addAttribute( "coord3d" );
	attribute_texcoord = m_shaderPr->addAttribute( "texcoord" );
	uniform_mvp = m_shaderPr->addUniform( "mvp" );
	m_unTextureRed = m_shaderPr->addUniform( "mytextureRed" );
	m_unTextureYellow = m_shaderPr->addUniform( "mytextureYellow" );

	std::cout.flush();
	//char f;
	//std::cin >> f;

	glEnable( GL_BLEND );
	glEnable(GL_DEPTH_TEST);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );


/*	glClearDepth( 1.0 );
	glDepthFunc( GL_LESS );
	glEnable( GL_DEPTH_TEST );
	glShadeModel( GL_SMOOTH );
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );

	setProjection( m_prType );*/
}

void CPlayState::Cleanup()
{
	delete m_RCube;
	delete m_shaderPr;

	glDeleteBuffers(1, &vbo_cube_vertices);
	glDeleteBuffers(1, &vbo_cube_texcoords);
	glDeleteBuffers(1, &ibo_cube_elements);
	glDeleteTextures(1, &m_textureRedID);

	MoveParams::cleanup();
	Colors::cleanup();
	AxisParams::cleanup();
}

void CPlayState::setProjection( const ProjectionType pType ) const
{
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	if ( pType == PT_DIMETRIC )
	{
		GLfloat cavalierPMatrix[ 16 ] = {
			1 , 0 , 0 , 0,
			0 , 1 , 0 , 0,
			0.3345, -0.3345, 1 , 0,
			0 , 0 , 0 , 1
		};

		glOrtho( -SCREEN_HORIZMARGIN, SCREEN_HORIZMARGIN, -SCREEN_VERTMARGIN, SCREEN_VERTMARGIN, 0.0, 40.0 );
		glMultMatrixf( cavalierPMatrix );
	}
	else if ( pType == PT_ISOMETRIC )
	{
		glOrtho( -SCREEN_HORIZMARGIN, SCREEN_HORIZMARGIN, -SCREEN_VERTMARGIN, SCREEN_VERTMARGIN, 0.0, 40.0 );
	};

	glMatrixMode( GL_MODELVIEW );
}

void CPlayState::Pause()
{

}

void CPlayState::Resume()
{

}

Point3D CPlayState::getGLPos( const int mX, const int mY ) const
{
	GLint viewport[ 4 ];
	glGetIntegerv( GL_VIEWPORT, viewport ); // 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT

	GLdouble modelview[16];
	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );

	GLdouble projection[ 16 ];
	glGetDoublev( GL_PROJECTION_MATRIX, projection );

	GLfloat winX, winY, winZ;
	winX = ( float ) mX;
	winY = ( float ) viewport[ 3 ] - ( float ) mY;
	glReadPixels( winX, winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

	GLdouble posX, posY, posZ;
	gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ );
//	std::cout << posX << " " << posY << " " << posZ << std::endl;
	return Point3D( posX, posY, posZ );
}

void CPlayState::HandleEvents( CGameEngine* game )
{
	static bool lastEvent = false;
	SDL_Event event;

	const Uint32 start = SDL_GetTicks();
	bool allEventsRunOut = false;

	while ( SDL_PollEvent( &event ) && !allEventsRunOut/* && ( SDL_GetTicks() - start ) < 15 */)
	{
		switch( event.type )
		{
		case SDL_QUIT:
			game->Quit();
			break;

		case SDL_WINDOWEVENT:
			if ( event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED )
			{
//				screen_width = width;
//				screen_height = height;
//				glViewport(0, 0, screen_width, screen_height);
			}
			break;

/*		case SDL_KEYDOWN:
			lastEvent = true;
			switch( event.key.keysym.sym )
			{
			case SDLK_ESCAPE:
				game->Quit();
				break;

			case SDLK_UP:
			case SDLK_w:
				m_gkStates[ GK_LOOKUP ].setDown();
				break;

			case SDLK_DOWN:
			case SDLK_s:
				m_gkStates[ GK_LOOKDOWN ].setDown();
				break;

			case SDLK_LEFT:
			case SDLK_a:
				m_gkStates[ GK_LOOKLEFT ].setDown();
				break;

			case SDLK_RIGHT:
			case SDLK_d:
				m_gkStates[ GK_LOOKRIGHT ].setDown();
				break;

			case SDLK_PAGEDOWN:
			case SDLK_e:
				m_gkStates[ GK_ROTATECOUNTERCLOCKWISE ].setDown();
				break;

			case SDLK_DELETE:
			case SDLK_q:
				m_gkStates[ GK_ROTATECLOCKWISE ].setDown();
				break;

			case SDLK_i:
				m_gkStates[ GK_MOVEFRONT ].setDown();
				break;

			case SDLK_u:
				m_gkStates[ GK_MOVEFRONTINV ].setDown();
				break;

			case SDLK_p:
				m_gkStates[ GK_MOVEBACK ].setDown();
				break;

			case SDLK_o:
				m_gkStates[ GK_MOVEBACKINV ].setDown();
				break;

			case SDLK_k:
				m_gkStates[ GK_MOVELEFT ].setDown();
				break;

			case SDLK_j:
				m_gkStates[ GK_MOVELEFTINV ].setDown();
				break;

			case SDLK_SEMICOLON:
				m_gkStates[ GK_MOVERIGHT ].setDown();
				break;

			case SDLK_l:
				m_gkStates[ GK_MOVERIGHTINV ].setDown();
				break;

			case SDLK_m:
				m_gkStates[ GK_MOVEUP ].setDown();
				break;

			case SDLK_n:
				m_gkStates[ GK_MOVEUPINV ].setDown();
				break;

			case SDLK_PERIOD:
				m_gkStates[ GK_MOVEDOWN ].setDown();
				break;

			case SDLK_COMMA:
				m_gkStates[ GK_MOVEDOWNINV ].setDown();
				break;

			case SDLK_SPACE:
				m_gkStates[ GK_CHANGECOLOR ].setDown();
				break;

			case SDLK_RETURN:
				m_gkStates[ GK_CHANGEPROJ ].setDown();
				break;
			}
			break;
//		case SDL_KEYUP:
//			break;
		case SDL_MOUSEBUTTONDOWN:
			switch( event.button.button )
			{
			case SDL_BUTTON_LEFT:
				m_pBegin = getGLPos( event.button.x, event.button.y );
//				std::cout << m_pBegin.x() << " " << m_pBegin.y() << " " << m_pBegin.z() << std::endl;

				break;
			case SDL_BUTTON_RIGHT:
				break;
			}
			break;
		case SDL_MOUSEBUTTONUP:
			switch( event.button.button )
			{
			case SDL_BUTTON_LEFT:
				m_pEnd = getGLPos( event.button.x, event.button.y );
//				std::cout << m_pEnd.x() << " " << m_pEnd.y() << " " << m_pEnd.z() << std::endl;

				m_gkStates[ GK_MOVEMOUSE ].setDown();
				m_gkStates[ GK_MOVEMOUSE ].releasePress();
				break;
			case SDL_BUTTON_RIGHT:
				break;
			}
			break;
		default:
			if ( lastEvent )
			{
				for ( int i = 0; i < GK_COUNT; ++i )
					m_gkStates[ i ].releasePress();

				m_pBegin.setXYZ( 0, 0, 0 );
				m_pEnd.setXYZ( 0, 0, 0 );

				lastEvent = false;
				allEventsRunOut = true;
			}
*/
			break;
		}
	}
}

void CPlayState::Update( CGameEngine * game )
{
/*	if ( !m_RCube->isRotating() && m_pBegin.is0() )
	{
        if ( m_gkStates[ GK_LOOKDOWN ].isNewDown() )
		{
			m_RCube->setRotates( 1, 0, 0, false );
			m_gkStates[ GK_LOOKDOWN ].releaseNewDown();
		}
		else if ( m_gkStates[ GK_LOOKUP ].isNewDown() )
		{
			m_RCube->setRotates( 1, 0, 0, true );
			m_gkStates[ GK_LOOKUP ].releaseNewDown();
		}
		else if ( m_gkStates[ GK_LOOKRIGHT ].isNewDown() )
		{
			m_RCube->setRotates( 0, 1, 0, false );
			m_gkStates[ GK_LOOKRIGHT ].releaseNewDown();
		}
		else if ( m_gkStates[ GK_LOOKLEFT ].isNewDown() )
		{
			m_RCube->setRotates( 0, 1, 0, true );
			m_gkStates[ GK_LOOKLEFT ].releaseNewDown();
		}
		else if ( m_gkStates[ GK_ROTATECOUNTERCLOCKWISE ].isNewDown() )
		{
			m_RCube->setRotates( 0, 0, 1, false );
			m_gkStates[ GK_ROTATECOUNTERCLOCKWISE ].releaseNewDown();
		}
		else if ( m_gkStates[ GK_ROTATECLOCKWISE ].isNewDown() )
		{
			m_RCube->setRotates( 0, 0, 1, true );
			m_gkStates[ GK_ROTATECLOCKWISE ].releaseNewDown();
		}
	}

	if ( !m_RCube->isMoving() && !m_RCube->isRotating() && !m_pBegin.is0() )
	{
		if ( m_gkStates[ GK_MOVEMOUSE ].isNewDown() )
		{
			m_RCube->setMoveByCoords( m_pBegin, m_pEnd );
			m_gkStates[ GK_MOVEMOUSE ].releaseNewDown();

			m_pBegin.setXYZ( 0, 0, 0 );
			m_pEnd.setXYZ( 0, 0, 0 );
		}
	}

	if ( !m_RCube->isMoving() && !m_RCube->isRotating() && m_pBegin.is0() )
	{
		for ( int i = 0; i < GK_MOVELAST - GK_MOVEFIRST + 1; i++ )
		{
			if ( m_gkStates[ GK_MOVEFRONT + i ].isNewDown() )
			{
				m_RCube->setMove( ( RCMoveType ) ( MT_FRONT + i ) );
				m_gkStates[ GK_MOVEFRONT + i ].releaseNewDown();
				break;
			}
		}
	}

	if ( m_gkStates[ GK_CHANGECOLOR ].isNewDown() )
	{
		RC::Colors::incScheme();
		m_gkStates[ GK_CHANGECOLOR ].releaseNewDown();
		m_needRedraw = true;
	}

	if ( m_gkStates[ GK_CHANGEPROJ ].isNewDown() )
	{
        m_prType = ProjectionType ( ( m_prType + 1 ) % PT_COUNT );
        setProjection( m_prType );
		m_gkStates[ GK_CHANGEPROJ ].releaseNewDown();
		m_needRedraw = true;
	}
*/
	float angle = SDL_GetTicks() / 2000.0 */** glm::radians(*/15.0;//);  // base 15° per second
	glm::mat4 anim =
		glm::rotate( glm::mat4( 1.0f ), angle * 3.0f, glm::vec3( 1, 0, 0 ) ) * // X axis
		glm::rotate( glm::mat4( 1.0f ), angle * 2.0f, glm::vec3( 0, 1, 0 ) ) * // Y axis
		glm::rotate( glm::mat4( 1.0f ), angle * 4.0f, glm::vec3( 0, 0, 1 ) );  // Z axis

	glm::mat4 model = glm::translate( glm::mat4( 1.0f ), glm::vec3( 0.0, 0.0, -4.0 ) );
	glm::mat4 view = glm::lookAt( glm::vec3( 0.0, 2.0, 0.0 ), glm::vec3( 0.0, 0.0, -4.0 ), glm::vec3( 0.0, 1.0, 0.0 ) );
	glm::mat4 projection = glm::perspective( 45.0f, 1.0f * SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 10.0f );

	glm::mat4 mvp = projection * view * model * anim;

	glUseProgram( m_shaderPr->id() );
	glUniformMatrix4fv( uniform_mvp, 1, GL_FALSE, glm::value_ptr( mvp ) );
}

void CPlayState::Draw( CGameEngine * game )
{
	static int drCount = 0;

//	if ( m_needRedraw || m_RCube->isRotating() || m_RCube->isMoving() )
	{
		Uint32 start = SDL_GetTicks();

		glClearColor( Colors::colR( RC_BG ), Colors::colG( RC_BG ), Colors::colB( RC_BG ), 0.0f );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		glUseProgram( m_shaderPr->id() );

	// begin
		glActiveTexture( GL_TEXTURE0 );
		glUniform1i( m_unTextureRed, /*GL_TEXTURE*/0 );
		glBindTexture( GL_TEXTURE_2D, m_textureRedID );

		glActiveTexture( GL_TEXTURE1 );
		glUniform1i( m_unTextureYellow, /*GL_TEXTURE*/1 );
		glBindTexture( GL_TEXTURE_2D, m_textureYellowID );

		glEnableVertexAttribArray( attribute_coord3d );
		// Describe our vertices array to OpenGL (it can't guess its format automatically)
		glBindBuffer( GL_ARRAY_BUFFER, vbo_cube_vertices );
		glVertexAttribPointer(
			attribute_coord3d, // attribute
			3,                 // number of elements per vertex, here (x,y,z)
			GL_FLOAT,          // the type of each element
			GL_FALSE,          // take our values as-is
			0,                 // no extra data between each position
			0                  // offset of first element
		);

		glEnableVertexAttribArray( attribute_texcoord );
		glBindBuffer( GL_ARRAY_BUFFER, vbo_cube_texcoords );
		glVertexAttribPointer(
			attribute_texcoord, // attribute
			2,                  // number of elements per vertex, here (x,y)
			GL_FLOAT,           // the type of each element
			GL_FALSE,           // take our values as-is
			0,                  // no extra data between each position
			0                   // offset of first element
		);

		/* Push each element in buffer_vertices to the vertex shader */
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements );
		int size;
		glGetBufferParameteriv( GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size );
		glDrawElements( GL_QUADS, size / sizeof( GLushort ), GL_UNSIGNED_SHORT, 0 );

		glDisableVertexAttribArray( attribute_coord3d );
		glDisableVertexAttribArray( attribute_texcoord );
	// ENd

/*		glLoadIdentity();

		if ( m_prType == PT_DIMETRIC )
		{
			glTranslatef( RC::CUBIE_COUNT - 2, -RC::CUBIE_COUNT + 2.5, -20 );
		}
		else if ( m_prType == PT_ISOMETRIC )
		{
			glTranslatef( 0.0f, 0.5, -20 );

			glRotatef( 35.264f, 1.0f, 0.0f, 0.0f );
			glRotatef( 45.0f, 0.0f, 1.0f, 0.0f );
		}
*/
//		m_RCube->rotateObject();
//		m_RCube->drawObject();

//		glFlush();

	//	if ( SDL_GetTicks() - start < SCREEN_TICK_PER_FRAME )
	//		SDL_Delay( SCREEN_TICK_PER_FRAME - ( SDL_GetTicks() - start ) );

		drCount++;
		if ( drCount > 1 ) m_needRedraw = false;
#ifdef MY_DEBUG
//		if ( drCount % 5 == 0 )
//			std::cout << "DrawCount: " << drCount << std::endl;
#endif // MY_DEBUG
	}
}

