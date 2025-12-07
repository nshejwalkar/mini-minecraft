#pragma once
#include "scene/chunk.h"
#include "scene/terrain.h"
#include <QRunnable>
#include <QMutex>

class VBOWorker : public QRunnable
{
private:
    Chunk* chunk;
    Terrain* terrain;
    QMutex* mutex;

public:
    VBOWorker(Chunk* chunk, Terrain* terrain, QMutex* mutex);  // per chunk
    void run() override;
};
