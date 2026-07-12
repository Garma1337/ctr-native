#ifndef CTR_NATIVE_REFERENCE_DUMP_H
#define CTR_NATIVE_REFERENCE_DUMP_H

#ifdef CTR_REFERENCE

struct Driver;
struct GameTracker;

/* Core capture: opens state.native.csv from --dump-state; sampled once per game-logic tick. */
void ReferenceDump_ConfigureFromArgs(int argc, char **argv);
void ReferenceDump_Tick(const struct GameTracker *gGT);

/* Offline: parse --export-replay <input.ctrreplay> into --dump-session <out.csv> without booting/replaying
 * (rebuild-safe). Returns 1 if handled so main() can exit before Platform_Init. */
int ReferenceDump_MaybeExportSession(int argc, char **argv);

void ReferenceDump_ConvertSpeedToVec(int speed, int axisRotationX, int axisRotationY, int velX, int velY, int velZ);
void ReferenceDump_OnGravity(const struct Driver *driver, int elapsedMs, int lowGravity, int velInX, int velInY, int velInZ, int actionsIn, int timerIn, int vShiftCountIn, int vShiftWindowIn, int firstFrameIn, int forwardDirIn, int velOutX, int velOutY, int velOutZ);
void ReferenceDump_StepHeadingTowardAxis(int angleIn, int axisIn, int elapsedMs, int angleOut, int axisOut);
void ReferenceDump_DesiredSpinRate(int steerInput, int driftDir, int steerVelStandard, int steerVelSwitchWay, int turnRate, int turnConst, int desiredSpinRate);
void ReferenceDump_ApproachDriftSpinRate(int spinIn, int desiredIn, int driftDir, int framesIn, int accel, int decel, int elapsedMs, int spinOut, int framesOut);
void ReferenceDump_TurnAngle(int spinIn, int driftTotalTimeMs, int rampFrames, int startupScale, int multDrift, int driftDir, int turnAngleIn, int driftTurnBase, int turnConst, int angleScale, int sameDirAngle, int oppDirAngle, int steerVelStd, int steerVelSwitch, int spinOut, int turnAngleOut);
void ReferenceDump_TurnWobble(int angleIn, int timerIn, int velIn, int framesDrifting, int framesTillSpinout, int driftTurnInput, int angleOut, int timerOut, int velOut);
void ReferenceDump_ApplyTurnToAngle(int angleIn, int signedSpinRate, int driftTurnInput, int elapsedMs, int ampTurnStateOut, int angleOut);
void ReferenceDump_TerrainSpeedScale(int speedIn, int baseSpeed, int multiplier, int braking, int baseSpeedOut, int scaledBaseOut);
void ReferenceDump_ResolveThrottle(int simpTurn, int speedApprox, int baseSpeed, int backwardSpeed, int square, int cross, int stickRY, int stickLY, int flagsIn, int speedOut, int flagsOut);
void ReferenceDump_AccelTap(int fireSpeedIn, int speed, int countIn, int windowIn, int windowMs, int kartState, int fireSpeedOut, int countOut, int windowOut);
void ReferenceDump_ResolveSteering(int turnRate, int turnConst, int accelTapCount, int accelTapSteerCount, int absSpeed, int wallRubTimer, int cross, int driverSpeed, int speedClassStat, int stickX, int rwdNull, int rwdCenter, int rwdDead, int rwdRange, int simpTurnIn, int numFramesIn, int flagsIn, int simpTurnOut, int flagsOut, int numFramesOut);
void ReferenceDump_FireSpeedCap(int fireLevel, int singleTurboSpeed, int sacredFireSpeed, int fireSpeedCap);
void ReferenceDump_Reserves(int type, int amount, int reservesIn, int outsideIn, int reservesOut, int outsideOut);
void ReferenceDump_HeadingSpin(int simpTurn, int forwardDirIn, int baseSpeed, int speedApprox, int flagsIn, int onGround, int onTurboPad, int desiredSpinOut, int forwardDirOut, int flagsOut);
void ReferenceDump_ApproachHeadingSpin(int desired, int current, int turnInputDelay, int turnConst, int turnResponseScale, int spinOut);
void ReferenceDump_AdvanceTurnAngle(int turnAngleCurrIn, int lerpVel, int turnAngleScale, int elapsedMs, int turnAngleOut);
void ReferenceDump_Barycentrics(int v1x, int v1y, int v1z, int v2x, int v2y, int v2z, int px, int py, int pz, int outx, int outy, int outz);
void ReferenceDump_TriangleNormal(int v1x, int v1y, int v1z, int v2x, int v2y, int v2z, int v3x, int v3y, int v3z, int lodShift, int scale, int normalShift, int nx, int ny, int nz, int halfDist, int axis);
void ReferenceDump_ModelVertex(const char *lodName, int animFrame, int mode, int vertexIndex, int sx, int sy, int sz, int ax, int ay, int az);
void ReferenceDump_ScrubImpact(const struct Driver *driver, int velInX, int velInY, int velInZ, int normalX, int normalY, int normalZ, int scrubDepth, int dot, int velOutX, int velOutY, int velOutZ);
void ReferenceDump_MovedStep(int driverID, int iter, int multiplier, int velX, int velY, int velZ, int stepX, int stepY, int stepZ, int hitFraction, int touched, int preX, int preY, int preZ, int postX, int postY, int postZ, int normalX, int normalY, int normalZ);
void ReferenceDump_AdvSpawnShuffle(unsigned int s0Pre, unsigned int s1Pre, int champBranch, const char *spawnOrder8);

#endif /* CTR_REFERENCE */
#endif /* CTR_NATIVE_REFERENCE_DUMP_H */
