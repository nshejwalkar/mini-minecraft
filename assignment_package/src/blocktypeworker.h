#pragma once
#include "scene/terrain.h"
#include <QRunnable>
#include <QMutex>


class BlockTypeWorker : public QRunnable
{
    friend class Terrain;
private:
    int zoneX, zoneZ;
    Terrain* terrain;
    QMutex* mutex;

public:
    BlockTypeWorker(int zoneX, int zoneZ, Terrain* terrain, QMutex* mutex);  // per zone
    void run() override;
};
