#include "mygl.h"
#include <glm_includes.h>
#include "constants.h"
#include "debug.h"
#include "scene/chunk.h"

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QDateTime>

#include <chrono>

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
    m_worldAxes(this), m_crosshair(this),
      m_progLambert(this), m_progFlat(this), m_progInstanced(this),
      postprocessNoOp(this), postprocessWater(this), postprocessLava(this),
      selectedPostProcessShader(&postprocessNoOp),
      postProcessFrameBuffer(this, this->width(), this->height(), (unsigned int)this->devicePixelRatio()),
      quad(this),
    m_terrain(this), m_player(STARTING_POS, m_terrain),
    last_time_polled(QDateTime::currentMSecsSinceEpoch()),
    last_mouse_pos(QPoint(width() / 2, height() / 2)),
    mouseDelta(0.f,0.f), textureAtlas(this),
    currTime(0.f),
    mouseRecenter(false), mouseLocked(false)
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
    // Tell the timer to redraw 60 times per second
    m_timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible
}

MyGL::~MyGL() {
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
}


void MyGL::moveMouseToCenter() {
    // qDebug() << QPoint(width() / 2, height() / 2);
    // qDebug() << this->mapToGlobal(QPoint(width() / 2, height() / 2));
    // qDebug();
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);

    // alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDepthFunc(GL_LEQUAL);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instance of the world axes
    m_worldAxes.createVBOdata();
    m_crosshair.createVBOdata();

    // create and set up the texture atlas
    textureAtlas.create(":/textures/minecraft_textures_all.png");  // default RGBA BGRA

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl",":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    m_progInstanced.create(":/glsl/instanced.vert.glsl", ":/glsl/lambert.frag.glsl");

    // Postprocess shaders
    postprocessNoOp.create(":/glsl/passthrough.vert.glsl", ":/glsl/noOp.frag.glsl");
    postprocessWater.create(":/glsl/passthrough.vert.glsl", ":/glsl/water.frag.glsl");
    postprocessLava.create(":/glsl/passthrough.vert.glsl", ":/glsl/lava.frag.glsl");

    // set the texture atlas slot now. it wont change
    m_progLambert.setUnifInt("u_Texture", 1);

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);

    // m_terrain.CreateTestScene();
    // m_terrain.loadChunks(m_player.mcr_position, true);
    // m_terrain.loadTGZ(m_player.mcr_position);
    m_terrain.loadSurroundingTGZs(m_player.mcr_position, true);
    lastBlockType = m_terrain.getGlobalBlockAt(STARTING_POS);

    quad.createVBOdata();
    postProcessFrameBuffer.create();
}


void MyGL::resizeGL(int w, int h) {
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_player.setCameraWidthHeight(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setUnifMat4("u_ViewProj", viewproj);
    m_progFlat.setUnifMat4("u_ViewProj", viewproj);
    m_progInstanced.setUnifMat4("u_ViewProj", viewproj);

    // Postprocess framebuffer
    postProcessFrameBuffer.resize(w, h, (unsigned int)this->devicePixelRatio());
    postProcessFrameBuffer.destroy();
    postProcessFrameBuffer.create();

    printGLErrorLog();
}


// MyGL's constructor links tick() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to perform
// all per-frame actions here, such as performing physics updates on all
// entities in the scene.
void MyGL::tick() {
    ++currTime;

    update(); // Calls paintGL() as part of a larger QOpenGLWidget pipeline

    auto cT = QDateTime::currentMSecsSinceEpoch();
    auto dT = cT - last_time_polled;
    m_inputs.mouseX = mouseDelta.x;
    m_inputs.mouseY = mouseDelta.y;
    // LOG("tick: " << m_inputs.mouseX << ", " << m_inputs.mouseY);

    m_player.tick(dT, this->m_inputs);  // qint64 -> float
    sendPlayerDataToGUI(); // Updates the info in the secondary window displaying player data

    // Get current block player is in
    try {
        currCameraBlock = m_terrain.getGlobalBlockAt(m_player.mcr_camera.mcr_position);
    }
    catch (const std::out_of_range& e) {
        currCameraBlock = EMPTY;
    }

    BlockType cb = m_terrain.getGlobalBlockAt(m_player.mcr_position);
    // LOG("current blocktype is" << static_cast<unsigned>(cb));

    // Load new chunks
    // m_terrain.loadChunks(m_player.mcr_position);
    // m_terrain.loadTGZ(m_player.mcr_position);
    m_terrain.loadSurroundingTGZs(m_player.mcr_position);

    m_inputs.tReleased = false;
    mouseDelta.x = 0.f;
    mouseDelta.y = 0.f;
    last_time_polled = cT;
}

void MyGL::sendPlayerDataToGUI() const {
    emit sig_sendPlayerPos(m_player.posAsQString());
    emit sig_sendPlayerVel(m_player.velAsQString());
    emit sig_sendPlayerAcc(m_player.accAsQString());
    emit sig_sendPlayerLook(m_player.lookAsQString());
    glm::vec2 pPos(m_player.mcr_position.x, m_player.mcr_position.z);
    glm::ivec2 chunk(16 * glm::ivec2(glm::floor(pPos / 16.f)));
    glm::ivec2 zone(64 * glm::ivec2(glm::floor(pPos / 64.f)));
    emit sig_sendPlayerChunk(QString::fromStdString("( " + std::to_string(chunk.x) + ", " + std::to_string(chunk.y) + " )"));
    emit sig_sendPlayerTerrainZone(QString::fromStdString("( " + std::to_string(zone.x) + ", " + std::to_string(zone.y) + " )"));
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL() {
    // Set postprocessing shader
    if (currCameraBlock == WATER) {
        selectedPostProcessShader = &postprocessWater;
    }
    else if (currCameraBlock == LAVA) {
        selectedPostProcessShader = &postprocessLava;
    }
    else {
        selectedPostProcessShader = &postprocessNoOp;
    }

    // Render postprocess framebuffer
    postProcessFrameBuffer.bindFrameBuffer();
    // glViewport(0, 0, width() * devicePixelRatio(), height() * devicePixelRatio());
    glViewport(0, 0, width(), height());

    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // update time
    m_progLambert.setUnifFloat("u_Time", currTime);

    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();
    m_progLambert.setUnifMat4("u_ViewProj", viewproj);
    m_progFlat.setUnifMat4("u_ViewProj", viewproj);
    m_progInstanced.setUnifMat4("u_ViewProj", viewproj);

    // rebind the atlas slot
    textureAtlas.bind(1);

    renderTerrain();

    glDisable(GL_DEPTH_TEST);
    m_progFlat.setUnifMat4("u_Model", glm::mat4());
    m_progFlat.draw(m_worldAxes);

    m_progFlat.setUnifMat4("u_ViewProj", glm::mat4());
    m_progFlat.setUnifMat4("u_Model", glm::mat4());
    m_progFlat.draw(m_crosshair);
    glEnable(GL_DEPTH_TEST);

    // Postprocessing
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    glViewport(0, 0, width() * devicePixelRatio(), height() * devicePixelRatio());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    postProcessFrameBuffer.bindToTextureSlot(0);
    selectedPostProcessShader->useMe();
    selectedPostProcessShader->setUnifInt("u_Texture", 0);
    selectedPostProcessShader->draw(quad);
}

// Change this so it renders the nine zones of generated
// terrain that surround the player (refer to Terrain::m_generatedTerrain
// for more info)
void MyGL::renderTerrain() {
    glm::vec3 playerPos = m_player.mcr_position;
    // int x = static_cast<int>(glm::floor(playerPos.x / 16.f)) * 16;
    // int z = static_cast<int>(glm::floor(playerPos.z / 16.f)) * 16;
    // m_terrain.draw(x - 512, x + 512, z - 512, z + 512, &m_progLambert);

    int zoneX = static_cast<int>(glm::floor(playerPos.x / 64.f));
    int zoneZ = static_cast<int>(glm::floor(playerPos.z / 64.f));

    int minX = (zoneX - RENDER_RADIUS) * 64;
    int maxX = (zoneX + RENDER_RADIUS + 1) * 64;
    int minZ = (zoneZ - RENDER_RADIUS) * 64;
    int maxZ = (zoneZ + RENDER_RADIUS + 1) * 64;

    m_terrain.draw(minX, maxX, minZ, maxZ, &m_progLambert);
}

void MyGL::lockMouse() {
    this->mouseLocked = !this->mouseLocked;
    if (!this->mouseLocked) {
        setMouseTracking(false);
        setCursor(Qt::ArrowCursor);
    } else {
        setMouseTracking(true);
        setCursor(Qt::BlankCursor);
    }
}

// not used or finished
// void MyGL::resetPlayer() {
//     m_player.resetEntity(STARTING_POS);
//     m_inputs.wPressed = false;
//     m_inputs.aPressed = false;
//     m_inputs.sPressed = false;
//     m_inputs.dPressed = false;
//     m_inputs.spacePressed = false;
//     m_inputs.mouseX = 0.0f;
//     m_inputs.mouseY = 0.0f;
// }

void MyGL::keyPressEvent(QKeyEvent *e) {
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }

    /*
        this function, keyReleaseEvent, mousePressEvent etc execute potentially 100s of times a second,
        independent of QTimer or any Qt mechanisms.
        camera is processed directly here for arrow keys.
        this function and mousePressEvent modify the InputBundle member variable m_inputs
        which is processed every 16 ms (rate set by the QTimer) by tick()
    */

    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_Right) {
        m_player.rotateOnUpGlobal(-amount);
    } else if (e->key() == Qt::Key_Left) {
        m_player.rotateOnUpGlobal(amount);
    } else if (e->key() == Qt::Key_Up) {
        m_player.rotateOnRightLocal(amount);
    } else if (e->key() == Qt::Key_Down) {
        m_player.rotateOnRightLocal(-amount);
    } else if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = true;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = true;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = true;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = true;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = true;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = true;
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = true;
    } else if (e->key() == Qt::Key_Shift) {
        m_inputs.shiftPressed = true;
    } else if (e->key() == Qt::Key_Control) {
        m_inputs.ctrlPressed = true;
    } else if (e->key() == Qt::Key_M) {
        this->lockMouse();
    } else if (e->key() == Qt::Key_F) {
        m_player.flight_mode = !m_player.flight_mode;
    } else if (e->key() == Qt::Key_P) {
        m_player.noclip_mode = !m_player.noclip_mode;
    }
}

void MyGL::keyReleaseEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = false;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = false;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = false;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = false;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = false;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = false;
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = false;
    } else if (e->key() == Qt::Key_Shift) {
        m_inputs.shiftPressed = false;
    } else if (e->key() == Qt::Key_Control) {
        m_inputs.ctrlPressed = false;
    } else if (e->key() == Qt::Key_T) {
        LOG("t released");
        m_inputs.tReleased = true;
    }
}

void MyGL::mouseMoveEvent(QMouseEvent *e) {
    // setMouseTracking is already true
    // qDebug() << "Mouse press at:" << e->pos()
    //          << "Button:" << e->button()
    //          << "Buttons held:" << e->buttons()
    //          << "Modifiers:" << e->modifiers();

    // calculate distance from center
    // auto start = std::chrono::steady_clock::now();

    QPoint center = QPoint(width() / 2, height() / 2);

    QPoint cP = e->pos();
    QPoint dP = cP-last_mouse_pos;
    last_mouse_pos = cP;

    // accumulate deltas every event, will be consumed and reset by tick() every frame
    mouseDelta.x += dP.x();
    mouseDelta.y += dP.y();

    QPoint dfc = cP - QPoint(width() / 2, height() / 2);
    // only recenter if we're 100 pixels awway from the center
    if (dfc.x()*dfc.x() + dfc.y()*dfc.y() > 10000) {
        // std::cout << "recentered mouse" << std::endl;
        this->moveMouseToCenter();
        last_mouse_pos = QPoint(width() / 2, height() / 2);
    }

    // auto end = std::chrono::steady_clock::now();
    // LOG("Took" << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "mus");
}

void MyGL::mousePressEvent(QMouseEvent *e) {
    qDebug() << "clicked at " << e->pos();
    glm::ivec3 out_hit;
    glm::ivec3 out_prevBlock;
    if (m_player.processClick(e, &out_hit, &out_prevBlock)) {
        if (e->button() == Qt::LeftButton) {
            BlockType block = m_terrain.getGlobalBlockAt(out_hit.x, out_hit.y, out_hit.z);
            if (block != BEDROCK) {
                m_terrain.setGlobalBlockAtUpdate(out_hit.x, out_hit.y, out_hit.z, EMPTY);
            }
        }
        else if (e->button() == Qt::RightButton) {
            m_terrain.setGlobalBlockAtUpdate(out_prevBlock.x, out_prevBlock.y, out_prevBlock.z, GRASS);
        }
    }
}
