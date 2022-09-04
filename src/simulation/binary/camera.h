#ifndef SIMULATION_BINARY_CAMERA_H
#define SIMULATION_BINARY_CAMERA_H

#include <foe/ecs/id.h>
#include <foe/imex/binary/exporter.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeSimulation foeSimulation;

foeResultSet export_foeCamera(foeEntityID entity,
                              foeSimulation const *pSimulation,
                              foeImexBinarySet *pBinarySet);

#ifdef __cplusplus
}
#endif

#endif // SIMULATION_BINARY_CAMERA_H