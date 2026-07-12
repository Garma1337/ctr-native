#ifdef CTR_REFERENCE

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "platform/native_input.h"
#include "platform/native_replay_scheduler.h"
#include "reference/reference_dump.h"

#define REFERENCE_DUMP_MAX_ROWS 200000

static FILE *ReferenceDump_OpenOnce(FILE **slot, const char *fileName, const char *header)
{
	if (*slot == NULL)
	{
		*slot = fopen(fileName, "w");
		if (*slot != NULL)
		{
			fprintf(*slot, "%s\n", header);
		}
	}
	return *slot;
}

void ReferenceDump_ConvertSpeedToVec(int speed, int axisRotationX, int axisRotationY, int velX, int velY, int velZ)
{
	static FILE *file;
	static int rowCount;

	if (rowCount >= REFERENCE_DUMP_MAX_ROWS)
	{
		return;
	}

	file = ReferenceDump_OpenOnce(&file, "convertspeedtovec.csv", "frame,speed,axisX,axisY,velX,velY,velZ");
	if (file != NULL)
	{
		fprintf(file, "%u,%d,%d,%d,%d,%d,%d\n", (unsigned int)NativeReplayScheduler_ReplayFrame(), speed, axisRotationX, axisRotationY, velX, velY, velZ);
		fflush(file);
		rowCount++;
	}
}

void ReferenceDump_OnGravity(const struct Driver *driver, int elapsedMs, int lowGravity, int velInX, int velInY, int velInZ, int actionsIn, int timerIn,
                             int vShiftCountIn, int vShiftWindowIn, int firstFrameIn, int forwardDirIn, int velOutX, int velOutY, int velOutZ)
{
	static FILE *file;
	static int rowCount;
	const MATRIX *m;

	if (rowCount >= REFERENCE_DUMP_MAX_ROWS)
	{
		return;
	}

	file = ReferenceDump_OpenOnce(&file, "ongravity.csv",
	                              "frame,m00,m01,m02,m10,m11,m12,m20,m21,m22,"
	                              "velInX,velInY,velInZ,elapsedMs,lowGravity,gravity,forwardAccelImpulse,"
	                              "actionsIn,speedApprox,baseSpeed,fireSpeed,slopeBonus,sideClamp,terminalVelocity,"
	                              "terrainFlags,kartState,actionsPrev,terrainScaledBaseSpeed,terrainSpeedMult,groundFrictionScale,"
	                              "rainCloud,noPedalPerp,noPedalFwd,pedalPerp,pedalFwd,brakeFriction,driftCurve,driftFriction,"
	                              "terrainFrictionBoost,timerIn,firstFrameRev,forwardDirIn,vShiftCountIn,vShiftWindowIn,"
	                              "velOutX,velOutY,velOutZ,actionsOut,timerOut,forwardDir,vShiftCountOut,vShiftWindowOut");
	if (file == NULL)
	{
		return;
	}

	m = &driver->matrixMovingDir;
	int ref_tmFlags = (driver->terrainMeta1 != NULL) ? (int)driver->terrainMeta1->flags : 0;
	int ref_tmGroundFric = (driver->terrainMeta1 != NULL) ? driver->terrainMeta1->groundFrictionScale : 0;
	int ref_tmSpeedMult = (driver->terrainMeta2 != NULL) ? driver->terrainMeta2->speedMultiplier : 0;
	fprintf(file,
	        "%u,"
	        "%d,%d,%d,%d,%d,%d,%d,%d,%d,"
	        "%d,%d,%d,%d,%d,%d,%d,"
	        "%d,%d,%d,%d,%d,%d,%d,"
	        "%d,%d,%d,%d,%d,%d,"
	        "%d,%d,%d,%d,%d,%d,%d,%d,"
	        "%d,%d,%d,%d,%d,%d,"
	        "%d,%d,%d,%d,%d,%d,%d,%d\n",
	        (unsigned int)NativeReplayScheduler_ReplayFrame(), m->m[0][0], m->m[0][1], m->m[0][2], m->m[1][0], m->m[1][1], m->m[1][2], m->m[2][0], m->m[2][1],
	        m->m[2][2], velInX, velInY, velInZ, elapsedMs, lowGravity, driver->const_Gravity, driver->forwardAccelImpulse, actionsIn, driver->speedApprox,
	        driver->baseSpeed, driver->fireSpeed, driver->const_SlopeForwardSpeedBonus, driver->const_SideSpeedClamp, driver->const_TerminalVelocity,
	        ref_tmFlags, driver->kartState, (int)driver->actionsFlagSetPrevFrame, driver->terrainScaledBaseSpeed, ref_tmSpeedMult, ref_tmGroundFric,
	        (int)driver->rainCloudEffect, driver->const_NoPedalFriction_Perpendicular, driver->const_NoPedalFriction_Forward,
	        driver->const_PedalFriction_Perpendicular, driver->const_PedalFriction_Forward, driver->const_BrakeFriction, driver->const_DriftCurve,
	        driver->const_DriftFriction, driver->const_TerrainFrictionBoost, timerIn, firstFrameIn, forwardDirIn, vShiftCountIn, vShiftWindowIn, velOutX,
	        velOutY, velOutZ, (int)driver->actionsFlagSet, driver->terrainFrictionTimer, driver->forwardDir, driver->vShiftCount, driver->vShiftWindowTimer);
	fflush(file);
	rowCount++;
}

void ReferenceDump_StepHeadingTowardAxis(int angleIn, int axisIn, int elapsedMs, int angleOut, int axisOut)
{
	static FILE *file;
	static int rowCount;

	if (rowCount >= REFERENCE_DUMP_MAX_ROWS)
	{
		return;
	}

	file = ReferenceDump_OpenOnce(&file, "stepheadingtowardaxis.csv", "angleIn,axisIn,elapsedMs,angleOut,axisOut");
	if (file != NULL)
	{
		fprintf(file, "%d,%d,%d,%d,%d\n", angleIn, axisIn, elapsedMs, angleOut, axisOut);
		fflush(file);
		rowCount++;
	}
}

void ReferenceDump_DesiredSpinRate(int steerInput, int driftDir, int steerVelStandard, int steerVelSwitchWay, int turnRate, int turnConst, int desiredSpinRate)
{
	static FILE *file;
	static int rowCount;

	if (rowCount >= REFERENCE_DUMP_MAX_ROWS)
	{
		return;
	}

	file = ReferenceDump_OpenOnce(&file, "desiredspinrate.csv", "steerInput,driftDir,steerVelStd,steerVelSwitch,turnRate,turnConst,desiredSpinRate");
	if (file != NULL)
	{
		fprintf(file, "%d,%d,%d,%d,%d,%d,%d\n", steerInput, driftDir, steerVelStandard, steerVelSwitchWay, turnRate, turnConst, desiredSpinRate);
		fflush(file);
		rowCount++;
	}
}

void ReferenceDump_ApproachDriftSpinRate(int spinIn, int desiredIn, int driftDir, int framesIn, int accel, int decel, int elapsedMs, int spinOut, int framesOut)
{
	static FILE *file;
	static int rowCount;

	if (rowCount >= REFERENCE_DUMP_MAX_ROWS)
	{
		return;
	}

	file = ReferenceDump_OpenOnce(&file, "approachdriftspinrate.csv", "spinIn,desiredIn,driftDir,framesIn,accel,decel,elapsedMs,spinOut,framesOut");
	if (file != NULL)
	{
		fprintf(file, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n", spinIn, desiredIn, driftDir, framesIn, accel, decel, elapsedMs, spinOut, framesOut);
		fflush(file);
		rowCount++;
	}
}

void ReferenceDump_TurnAngle(int spinIn, int driftTotalTimeMs, int rampFrames, int startupScale, int multDrift, int driftDir, int turnAngleIn,
                             int driftTurnBase, int turnConst, int angleScale, int sameDirAngle, int oppDirAngle, int steerVelStd, int steerVelSwitch,
                             int spinOut, int turnAngleOut)
{
	static FILE *file;
	static int rowCount;

	if (rowCount >= REFERENCE_DUMP_MAX_ROWS)
	{
		return;
	}

	file = ReferenceDump_OpenOnce(&file, "turnangle.csv",
	                              "spinIn,driftTotalTimeMs,rampFrames,startupScale,multDrift,driftDir,turnAngleIn,driftTurnBase,turnConst,"
	                              "angleScale,sameDirAngle,oppDirAngle,steerVelStd,steerVelSwitch,spinOut,turnAngleOut");
	if (file != NULL)
	{
		fprintf(file, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", spinIn, driftTotalTimeMs, rampFrames, startupScale, multDrift, driftDir, turnAngleIn,
		        driftTurnBase, turnConst, angleScale, sameDirAngle, oppDirAngle, steerVelStd, steerVelSwitch, spinOut, turnAngleOut);
		fflush(file);
		rowCount++;
	}
}

void ReferenceDump_TurnWobble(int angleIn, int timerIn, int velIn, int framesDrifting, int framesTillSpinout, int driftTurnInput, int angleOut, int timerOut,
                              int velOut)
{
	static FILE *file;
	static int rowCount;

	if (rowCount >= REFERENCE_DUMP_MAX_ROWS)
	{
		return;
	}

	file = ReferenceDump_OpenOnce(&file, "turnwobble.csv", "angleIn,timerIn,velIn,framesDrifting,framesTillSpinout,driftTurnInput,angleOut,timerOut,velOut");
	if (file != NULL)
	{
		fprintf(file, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n", angleIn, timerIn, velIn, framesDrifting, framesTillSpinout, driftTurnInput, angleOut, timerOut, velOut);
		fflush(file);
		rowCount++;
	}
}

void ReferenceDump_ApplyTurnToAngle(int angleIn, int signedSpinRate, int driftTurnInput, int elapsedMs, int ampTurnStateOut, int angleOut)
{
	static FILE *file;
	static int rowCount;

	if (rowCount >= REFERENCE_DUMP_MAX_ROWS)
	{
		return;
	}

	file = ReferenceDump_OpenOnce(&file, "applyturntoangle.csv", "angleIn,signedSpinRate,driftTurnInput,elapsedMs,ampTurnStateOut,angleOut");
	if (file != NULL)
	{
		fprintf(file, "%d,%d,%d,%d,%d,%d\n", angleIn, signedSpinRate, driftTurnInput, elapsedMs, ampTurnStateOut, angleOut);
		fflush(file);
		rowCount++;
	}
}

void ReferenceDump_TerrainSpeedScale(int speedIn, int baseSpeed, int multiplier, int braking, int baseSpeedOut, int scaledBaseOut)
{
	static FILE *file;
	static int rowCount;

	if (rowCount >= REFERENCE_DUMP_MAX_ROWS)
	{
		return;
	}

	file = ReferenceDump_OpenOnce(&file, "terrainspeedscale.csv", "speedIn,baseSpeed,multiplier,braking,baseSpeedOut,scaledBaseOut");
	if (file != NULL)
	{
		fprintf(file, "%d,%d,%d,%d,%d,%d\n", speedIn, baseSpeed, multiplier, braking, baseSpeedOut, scaledBaseOut);
		fflush(file);
		rowCount++;
	}
}

void ReferenceDump_ResolveThrottle(int simpTurn, int speedApprox, int baseSpeed, int backwardSpeed, int square, int cross, int stickRY, int stickLY,
                                   int flagsIn, int speedOut, int flagsOut)
{
	static FILE *file;
	static int rowCount;

	if (rowCount >= REFERENCE_DUMP_MAX_ROWS)
	{
		return;
	}

	file = ReferenceDump_OpenOnce(&file, "resolvethrottle.csv",
	                              "simpTurn,speedApprox,baseSpeed,backwardSpeed,square,cross,stickRY,stickLY,flagsIn,speedOut,flagsOut");
	if (file != NULL)
	{
		fprintf(file, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", simpTurn, speedApprox, baseSpeed, backwardSpeed, square, cross, stickRY, stickLY, flagsIn, speedOut,
		        flagsOut);
		fflush(file);
		rowCount++;
	}
}

void ReferenceDump_AccelTap(int fireSpeedIn, int speed, int countIn, int windowIn, int windowMs, int kartState, int fireSpeedOut, int countOut, int windowOut)
{
	static FILE *file;
	static int rowCount;

	if (rowCount >= REFERENCE_DUMP_MAX_ROWS)
	{
		return;
	}

	file = ReferenceDump_OpenOnce(&file, "acceltap.csv", "fireSpeedIn,speed,countIn,windowIn,windowMs,kartState,fireSpeedOut,countOut,windowOut");
	if (file != NULL)
	{
		fprintf(file, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n", fireSpeedIn, speed, countIn, windowIn, windowMs, kartState, fireSpeedOut, countOut, windowOut);
		fflush(file);
		rowCount++;
	}
}

void ReferenceDump_ResolveSteering(int turnRate, int turnConst, int accelTapCount, int accelTapSteerCount, int absSpeed, int wallRubTimer, int cross,
                                   int driverSpeed, int speedClassStat, int stickX, int rwdNull, int rwdCenter, int rwdDead, int rwdRange, int simpTurnIn,
                                   int numFramesIn, int flagsIn, int simpTurnOut, int flagsOut, int numFramesOut)
{
	static FILE *file;
	static int rowCount;

	if (rowCount >= REFERENCE_DUMP_MAX_ROWS)
	{
		return;
	}

	file = ReferenceDump_OpenOnce(&file, "resolvesteering.csv",
	                              "turnRate,turnConst,accelTapCount,accelTapSteerCount,absSpeed,wallRubTimer,cross,driverSpeed,speedClassStat,"
	                              "stickX,rwdNull,rwdCenter,rwdDead,rwdRange,simpTurnIn,numFramesIn,flagsIn,simpTurnOut,flagsOut,numFramesOut");
	if (file != NULL)
	{
		fprintf(file, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", turnRate, turnConst, accelTapCount, accelTapSteerCount, absSpeed,
		        wallRubTimer, cross, driverSpeed, speedClassStat, stickX, rwdNull, rwdCenter, rwdDead, rwdRange, simpTurnIn, numFramesIn, flagsIn, simpTurnOut,
		        flagsOut, numFramesOut);
		fflush(file);
		rowCount++;
	}
}

void ReferenceDump_FireSpeedCap(int fireLevel, int singleTurboSpeed, int sacredFireSpeed, int fireSpeedCap)
{
	static FILE *file;
	static int rowCount;

	if (rowCount >= REFERENCE_DUMP_MAX_ROWS)
	{
		return;
	}

	file = ReferenceDump_OpenOnce(&file, "firespeedcap.csv", "fireLevel,singleTurboSpeed,sacredFireSpeed,fireSpeedCap");
	if (file != NULL)
	{
		fprintf(file, "%d,%d,%d,%d\n", fireLevel, singleTurboSpeed, sacredFireSpeed, fireSpeedCap);
		fflush(file);
		rowCount++;
	}
}

void ReferenceDump_Reserves(int type, int amount, int reservesIn, int outsideIn, int reservesOut, int outsideOut)
{
	static FILE *file;
	static int rowCount;

	if (rowCount >= REFERENCE_DUMP_MAX_ROWS)
	{
		return;
	}

	file = ReferenceDump_OpenOnce(&file, "reserves.csv", "type,amount,reservesIn,outsideIn,reservesOut,outsideOut");
	if (file != NULL)
	{
		fprintf(file, "%d,%d,%d,%d,%d,%d\n", type, amount, reservesIn, outsideIn, reservesOut, outsideOut);
		fflush(file);
		rowCount++;
	}
}

void ReferenceDump_HeadingSpin(int simpTurn, int forwardDirIn, int baseSpeed, int speedApprox, int flagsIn, int onGround, int onTurboPad, int desiredSpinOut,
                               int forwardDirOut, int flagsOut)
{
	static FILE *file;
	static int rowCount;

	if (rowCount >= REFERENCE_DUMP_MAX_ROWS)
	{
		return;
	}

	file = ReferenceDump_OpenOnce(&file, "headingspin.csv",
	                              "simpTurn,forwardDirIn,baseSpeed,speedApprox,flagsIn,onGround,onTurboPad,desiredSpinOut,forwardDirOut,flagsOut");
	if (file != NULL)
	{
		fprintf(file, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", simpTurn, forwardDirIn, baseSpeed, speedApprox, flagsIn, onGround, onTurboPad, desiredSpinOut,
		        forwardDirOut, flagsOut);
		fflush(file);
		rowCount++;
	}
}

void ReferenceDump_ApproachHeadingSpin(int desired, int current, int turnInputDelay, int turnConst, int turnResponseScale, int spinOut)
{
	static FILE *file;
	static int rowCount;

	if (rowCount >= REFERENCE_DUMP_MAX_ROWS)
	{
		return;
	}

	file = ReferenceDump_OpenOnce(&file, "approachheadingspin.csv", "desired,current,turnInputDelay,turnConst,turnResponseScale,spinOut");
	if (file != NULL)
	{
		fprintf(file, "%d,%d,%d,%d,%d,%d\n", desired, current, turnInputDelay, turnConst, turnResponseScale, spinOut);
		fflush(file);
		rowCount++;
	}
}

void ReferenceDump_AdvanceTurnAngle(int turnAngleCurrIn, int lerpVel, int turnAngleScale, int elapsedMs, int turnAngleOut)
{
	static FILE *file;
	static int rowCount;

	if (rowCount >= REFERENCE_DUMP_MAX_ROWS)
	{
		return;
	}

	file = ReferenceDump_OpenOnce(&file, "advanceturnangle.csv", "turnAngleCurrIn,lerpVel,turnAngleScale,elapsedMs,turnAngleOut");
	if (file != NULL)
	{
		fprintf(file, "%d,%d,%d,%d,%d\n", turnAngleCurrIn, lerpVel, turnAngleScale, elapsedMs, turnAngleOut);
		fflush(file);
		rowCount++;
	}
}

void ReferenceDump_Barycentrics(int v1x, int v1y, int v1z, int v2x, int v2y, int v2z, int px, int py, int pz, int outx, int outy, int outz)
{
	static FILE *file;
	static int rowCount;

	if (rowCount >= REFERENCE_DUMP_MAX_ROWS)
	{
		return;
	}

	file = ReferenceDump_OpenOnce(&file, "barycentrics.csv", "v1x,v1y,v1z,v2x,v2y,v2z,px,py,pz,outx,outy,outz");
	if (file != NULL)
	{
		fprintf(file, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", v1x, v1y, v1z, v2x, v2y, v2z, px, py, pz, outx, outy, outz);
		fflush(file);
		rowCount++;
	}
}

void ReferenceDump_TriangleNormal(int v1x, int v1y, int v1z, int v2x, int v2y, int v2z, int v3x, int v3y, int v3z, int lodShift, int scale, int normalShift,
                                  int nx, int ny, int nz, int halfDist, int axis)
{
	static FILE *file;
	static int rowCount;

	if (rowCount >= REFERENCE_DUMP_MAX_ROWS)
	{
		return;
	}

	file = ReferenceDump_OpenOnce(&file, "trianglenormal.csv", "v1x,v1y,v1z,v2x,v2y,v2z,v3x,v3y,v3z,lodShift,scale,normalShift,nx,ny,nz,halfDist,axis");
	if (file != NULL)
	{
		fprintf(file, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z, lodShift, scale, normalShift, nx, ny,
		        nz, halfDist, axis);
		fflush(file);
		rowCount++;
	}
}

void ReferenceDump_ModelVertex(const char *lodName, int animFrame, int mode, int vertexIndex, int sx, int sy, int sz, int ax, int ay, int az)
{
	static FILE *file;
	static int done;
	static int started;
	static int lastVi;

	if (done || lodName == NULL || strcmp(lodName, "crash_hi") != 0)
	{
		return;
	}

	if (started && vertexIndex <= lastVi)
	{
		done = 1;
		return;
	}

	file = ReferenceDump_OpenOnce(&file, "modelvertices.csv", "lod,animFrame,mode,vertexIndex,sx,sy,sz,ax,ay,az");
	if (file == NULL)
	{
		return;
	}

	fprintf(file, "%s,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", lodName, animFrame, mode, vertexIndex, sx, sy, sz, ax, ay, az);
	fflush(file);
	started = 1;
	lastVi = vertexIndex;
}

void ReferenceDump_ScrubImpact(const struct Driver *driver, int velInX, int velInY, int velInZ, int normalX, int normalY, int normalZ, int scrubDepth, int dot,
                               int velOutX, int velOutY, int velOutZ)
{
	static FILE *file;
	static int rowCount;
	const MATRIX *m;

	if (rowCount >= REFERENCE_DUMP_MAX_ROWS)
	{
		return;
	}

	file = ReferenceDump_OpenOnce(&file, "scrubimpact.csv",
	                              "frame,driverID,m02,m12,m22,velInX,velInY,velInZ,normalX,normalY,normalZ,scrubDepth,dot,velOutX,velOutY,velOutZ");
	if (file != NULL)
	{
		m = &driver->matrixMovingDir;
		fprintf(file, "%u,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", (unsigned int)NativeReplayScheduler_ReplayFrame(), (int)driver->driverID, m->m[0][2],
		        m->m[1][2], m->m[2][2], velInX, velInY, velInZ, normalX, normalY, normalZ, scrubDepth, dot, velOutX, velOutY, velOutZ);
		fflush(file);
		rowCount++;
	}
}

void ReferenceDump_MovedStep(int driverID, int iter, int multiplier, int velX, int velY, int velZ, int stepX, int stepY, int stepZ, int hitFraction,
                             int touched, int preX, int preY, int preZ, int postX, int postY, int postZ, int normalX, int normalY, int normalZ)
{
	static FILE *file;
	static int rowCount;

	if (rowCount >= REFERENCE_DUMP_MAX_ROWS)
	{
		return;
	}

	file = ReferenceDump_OpenOnce(&file, "movedstep.csv",
	                              "frame,driverID,iter,mult,velX,velY,velZ,stepX,stepY,stepZ,hitFraction,touched,preX,preY,preZ,postX,postY,postZ,nX,nY,nZ");
	if (file != NULL)
	{
		fprintf(file, "%u,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", (unsigned int)NativeReplayScheduler_ReplayFrame(), driverID, iter,
		        multiplier, velX, velY, velZ, stepX, stepY, stepZ, hitFraction, touched, preX, preY, preZ, postX, postY, postZ, normalX, normalY, normalZ);
		fflush(file);
		rowCount++;
	}
}

void ReferenceDump_AdvSpawnShuffle(unsigned int s0Pre, unsigned int s1Pre, int champBranch, const char *spawnOrder8)
{
	static FILE *file;
	static int rowCount;

	if (rowCount >= REFERENCE_DUMP_MAX_ROWS)
	{
		return;
	}

	file = ReferenceDump_OpenOnce(&file, "advspawn.csv", "frame,s0Pre,s1Pre,champBranch,slot0,slot1,slot2,slot3,slot4,slot5,slot6,slot7");
	if (file != NULL)
	{
		fprintf(file, "%u,%u,%u,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", (unsigned int)NativeReplayScheduler_ReplayFrame(), s0Pre, s1Pre, champBranch,
		        (int)spawnOrder8[0], (int)spawnOrder8[1], (int)spawnOrder8[2], (int)spawnOrder8[3], (int)spawnOrder8[4], (int)spawnOrder8[5],
		        (int)spawnOrder8[6], (int)spawnOrder8[7]);
		fflush(file);
		rowCount++;
	}
}

static FILE *s_referenceStateFile;
static FILE *s_referenceFullFile;
static FILE *s_referenceSessionFile;
static FILE *s_referenceItemsFile;
static FILE *s_referenceVisibilityFile;
static int s_referenceSessionHeaderWritten;
static int s_referenceRaceStarted;
static unsigned int s_referenceRaceStartFrame;

static int ReferenceDump_MapPsxButtons(unsigned int psxActiveHigh)
{
	static const struct
	{
		unsigned int psx;
		int sharp;
	} kMap[] = {
	    {0x4000, 0x0201} {0x2000, 0x0208},
	    {0x8000, 0x0402},
	    {0x1000, 0x0400},
	    {0x0400, 0x0804},
	    {0x0800, 0x1004},
	    {0x0080, 0x0010},
	    {0x0020, 0x0020},
	    {0x0010, 0x0040},
	    {0x0040, 0x0080},
	    {0x0008, 0x0100},
	    {0x0200, 0x2000},
	    {0x0100, 0x4000},
	};
	int out = 0;
	int i;
	for (i = 0; i < (int)(sizeof(kMap) / sizeof(kMap[0])); i++)
	{
		if ((psxActiveHigh & kMap[i].psx) != 0)
		{
			out |= kMap[i].sharp;
		}
	}
	return out;
}

static int ReferenceDump_MapSteer(unsigned int leftStickX)
{
	int axis = ((int)leftStickX - 0x80) * 256;
	return -(axis * 4096) / 32767;
}

_Static_assert(sizeof(struct Driver) == 1592, "Driver layout drifted from reference full.bin schema (expected 1592)");
_Static_assert(sizeof(struct GameTracker) == 9612, "GameTracker layout drifted from reference full.bin schema (expected 9612)");

static void ReferenceDump_WriteSchema(const char *path)
{
	FILE *schema = fopen(path, "w");
	if (schema == NULL)
	{
		return;
	}
	fprintf(schema, "# struct\tfield\toffset\tsize\ttype\n");
	fprintf(schema, "# meta\tsizeofDriver\t%u\n", (unsigned int)sizeof(struct Driver));
	fprintf(schema, "# meta\tsizeofGameTracker\t%u\n", (unsigned int)sizeof(struct GameTracker));
#define REF_FIELD(f, t) \
	fprintf(schema, "Driver\t" #f "\t%u\t%u\t" t "\n", (unsigned int)offsetof(struct Driver, f), (unsigned int)sizeof(((struct Driver *)0)->f))
#define REF_FIELD_GT(f, t) \
	fprintf(schema, "GameTracker\t" #f "\t%u\t%u\t" t "\n", (unsigned int)offsetof(struct GameTracker, f), (unsigned int)sizeof(((struct GameTracker *)0)->f))
	REF_FIELD(driverID, "u8");
	REF_FIELD(kartState, "u8");
	REF_FIELD(lapIndex, "u8");
	REF_FIELD(heldItemID, "u8");
	REF_FIELD(posCurr.x, "s32");
	REF_FIELD(posCurr.y, "s32");
	REF_FIELD(posCurr.z, "s32");
	REF_FIELD(velocity.x, "s32");
	REF_FIELD(velocity.y, "s32");
	REF_FIELD(velocity.z, "s32");
	REF_FIELD(angle, "s16");
	REF_FIELD(axisRotationX, "s16");
	REF_FIELD(axisRotationY, "s16");
	REF_FIELD(speed, "s16");
	REF_FIELD(speedApprox, "s16");
	REF_FIELD(baseSpeed, "s16");
	REF_FIELD(fireSpeed, "s16");
	REF_FIELD(fireSpeedCap, "s16");
	REF_FIELD(terrainScaledBaseSpeed, "s16");
	REF_FIELD(forwardDir, "s16");
	REF_FIELD(forwardAccelImpulse, "s16");
	REF_FIELD(turboConst, "s8");
	REF_FIELD(accel.x, "s32");
	REF_FIELD(accel.y, "s32");
	REF_FIELD(accel.z, "s32");
	REF_FIELD(rotCurr.x, "s16");
	REF_FIELD(rotCurr.y, "s16");
	REF_FIELD(rotCurr.z, "s16");
	REF_FIELD(rotCurr.w, "s16");
	REF_FIELD(rotPrev.x, "s16");
	REF_FIELD(rotPrev.y, "s16");
	REF_FIELD(rotPrev.z, "s16");
	REF_FIELD(rotPrev.w, "s16");
	REF_FIELD(AxisAngle2_normalVec.x, "s16");
	REF_FIELD(AxisAngle2_normalVec.y, "s16");
	REF_FIELD(AxisAngle2_normalVec.z, "s16");
	REF_FIELD(AxisAngle3_normalVec.x, "s16");
	REF_FIELD(AxisAngle3_normalVec.y, "s16");
	REF_FIELD(AxisAngle3_normalVec.z, "s16");
	REF_FIELD(turnWobbleAngle, "s16");
	REF_FIELD(turnWobbleVelocity, "s16");
	REF_FIELD(turnWobbleTimer, "s16");
	REF_FIELD(rotationSpinRate, "s16");
	REF_FIELD(ampTurnState, "s16");
	REF_FIELD(numFramesSpentSteering, "s16");
	REF_FIELD(const_SteerAccel_Stage2_FirstFrame, "s8");
	REF_FIELD(const_SteerAccel_Stage2_FrameLength, "s8");
	REF_FIELD(const_SteerAccel_Stage4_FirstFrame, "s8");
	REF_FIELD(const_SteerAccel_Stage1_MaxSteer, "s16");
	REF_FIELD(const_SteerAccel_Stage1_MinSteer, "s16");
	REF_FIELD(const_SteerAccelTurnVelScale, "s16");
	REF_FIELD(const_SteerAccelTurnVelLimit, "s16");
	REF_FIELD(actionsFlagSet, "u32");
	REF_FIELD(actionsFlagSetPrevFrame, "u32");
	REF_FIELD(collisionFlags, "u16");
	REF_FIELD(stepFlagSet, "u32");
	REF_FIELD(reserves, "s16");
	REF_FIELD(turbo_outsideTimer, "s16");
	REF_FIELD(turbo_MeterRoomLeft, "s16");
	REF_FIELD(wallRubTimer, "s16");
	REF_FIELD(wallRubSpeedLimit, "s16");
	REF_FIELD(burnTimer, "s16");
	REF_FIELD(superEngineTimer, "s16");
	REF_FIELD(itemRollTimer, "s16");
	REF_FIELD(vShiftCount, "s16");
	REF_FIELD(vShiftWindowTimer, "s16");
	REF_FIELD(vShiftStartGuardTimer, "s16");
	REF_FIELD(jump_HighJumpTimerMS, "s16");
	REF_FIELD(simpTurnState, "s8");
	REF_FIELD(multDrift, "s16");
	REF_FIELD(jump_LandingBoost, "s16");
	REF_FIELD(jumpHeightCurr, "s16");
	REF_FIELD(jumpHeightPrev, "s16");
	REF_FIELD(KartStates.Drifting.numFramesDrifting, "s16");
	REF_FIELD(KartStates.Drifting.driftBoostTimeMS, "s16");
	REF_FIELD(terrainFrictionTimer, "s16");
	REF_FIELD(KartStates.Drifting.driftTotalTimeMS, "s16");
	REF_FIELD(KartStates.Drifting.numBoostsAttempted, "s8");
	REF_FIELD(KartStates.Drifting.numBoostsSuccess, "s8");
	REF_FIELD(jumpMeter, "s16");
	REF_FIELD(jumpMeterTimer, "s16");
	REF_FIELD(jump_TenBuffer, "s16");
	REF_FIELD(jump_CooldownMS, "s16");
	REF_FIELD(jump_CoyoteTimerMS, "s16");
	REF_FIELD(accelTapCount, "s16");
	REF_FIELD(accelTapWindowTimer, "s16");
	REF_FIELD(checkpoint.branchChoiceIndex, "u8");
	REF_FIELD(checkpoint.currentIndex, "u8");
	REF_FIELD(KartStates.RevEngine.overRevTimerMS, "s16");
	REF_FIELD(KartStates.RevEngine.releaseCooldownTimerMS, "s16");
	REF_FIELD(KartStates.RevEngine.emptyCooldownTimerMS, "s16");
	REF_FIELD(KartStates.RevEngine.chargeState, "u8");
	REF_FIELD(KartStates.RevEngine.lockoutFlags, "u8");
	REF_FIELD(quadBlockHeight, "s32");
	REF_FIELD(posWallColl.x, "s16");
	REF_FIELD(posWallColl.y, "s16");
	REF_FIELD(posWallColl.z, "s16");
	REF_FIELD(spsHitPos.x, "s16");
	REF_FIELD(spsHitPos.y, "s16");
	REF_FIELD(spsHitPos.z, "s16");
	REF_FIELD(spsNormalVec.x, "s16");
	REF_FIELD(spsNormalVec.y, "s16");
	REF_FIELD(spsNormalVec.z, "s16");
	REF_FIELD(rainCloudEffect, "u8");
	REF_FIELD(pendingDamageType, "u8");
	REF_FIELD(numTimesWumpa, "u8");
	REF_FIELD(botData.botPath, "s16");
	REF_FIELD(botData.navProgressRemainder, "s32");
	REF_FIELD(botData.botFlags, "u32");
	REF_FIELD(botData.ai_progress_cooldown, "s32");
	REF_FIELD(botData.botAccel, "s32");
	REF_FIELD(botData.aiDamageState, "s16");
	REF_FIELD(botData.ai_quadblock_checkpointIndex, "u8");
	REF_FIELD(botData.weaponCooldown, "s16");
	REF_FIELD(botData.aiPhysics.driftTarget, "s16");
	REF_FIELD(botData.aiPhysics.mulDrift, "s16");
	REF_FIELD(botData.aiPhysics.simpTurnState, "s16");
	REF_FIELD(botData.aiPhysics.turboMeter, "s16");
	REF_FIELD(botData.aiPhysics.fireLevel, "s16");
	REF_FIELD(botData.aiPhysics.speedY, "s32");
	REF_FIELD(botData.aiPhysics.speedLinear, "s32");
	REF_FIELD_GT(cameraDC[0].cameraPos.x, "s32");
	REF_FIELD_GT(cameraDC[0].cameraPos.y, "s32");
	REF_FIELD_GT(cameraDC[0].cameraPos.z, "s32");
	REF_FIELD_GT(cameraDC[0].lookAtPos.x, "s32");
	REF_FIELD_GT(cameraDC[0].lookAtPos.y, "s32");
	REF_FIELD_GT(cameraDC[0].lookAtPos.z, "s32");
	REF_FIELD_GT(cameraDC[0].mode, "u16");
	REF_FIELD_GT(cameraDC[0].cameraMode, "s16");
	REF_FIELD_GT(cameraDC[0].flags, "u32");
	REF_FIELD_GT(cameraDC[0].transitionBlend, "s16");
	REF_FIELD_GT(pushBuffer[0].rot.x, "s16");
	REF_FIELD_GT(pushBuffer[0].rot.y, "s16");
	REF_FIELD_GT(pushBuffer[0].rot.z, "s16");
#undef REF_FIELD_GT
#undef REF_FIELD
	fclose(schema);
}

static const char *ReferenceDump_ArgValue(int argc, char **argv, const char *name)
{
	int i;
	for (i = 1; i < argc - 1; i++)
	{
		if (strcmp(argv[i], name) == 0)
		{
			return argv[i + 1];
		}
	}
	return NULL;
}

static void ReferenceDump_WriteSessionRow(unsigned int frame, int dtMs, int rng)
{
	struct PlatformInputPadSnapshot pads[PLATFORM_INPUT_PAD_COUNT];
	int slot;

	if (s_referenceSessionHeaderWritten == 0)
	{
		s_referenceSessionHeaderWritten = 1;
		fprintf(s_referenceSessionFile, "CTRSESSION,version=2,rngSeed=%d,gamepadSlots=8,frameCount=,raceStartFrame=\n", rng);
		fprintf(s_referenceSessionFile, "frame,dtMs");
		for (slot = 0; slot < 8; slot++)
		{
			fprintf(s_referenceSessionFile, ",p%d_buttons,p%d_steer", slot, slot);
		}
		fprintf(s_referenceSessionFile, "\n");

		{
			unsigned int f;
			for (f = 0; f < frame; f++)
			{
				fprintf(s_referenceSessionFile, "%u,32", f);
				for (slot = 0; slot < 8; slot++)
				{
					fprintf(s_referenceSessionFile, ",0,0");
				}
				fprintf(s_referenceSessionFile, "\n");
			}
		}
	}

	Platform_InputCapturePadSnapshots(pads, PLATFORM_INPUT_PAD_COUNT);

	fprintf(s_referenceSessionFile, "%u,%d", frame, dtMs);
	for (slot = 0; slot < 8; slot++)
	{
		int buttons = 0;
		int steer = 0;
		if (slot < PLATFORM_INPUT_PAD_COUNT && pads[slot].connected != 0)
		{
			unsigned int psxLow = (unsigned int)(pads[slot].buttons[0] | (pads[slot].buttons[1] << 8));
			unsigned int psxActiveHigh = (~psxLow) & 0xFFFF;
			buttons = ReferenceDump_MapPsxButtons(psxActiveHigh);
			steer = ReferenceDump_MapSteer(pads[slot].analog[2]);
		}
		fprintf(s_referenceSessionFile, ",%d,%d", buttons, steer);
	}
	fprintf(s_referenceSessionFile, "\n");
	fflush(s_referenceSessionFile);
}

static void ReferenceDump_WriteFrameInfoFields(FILE *out, const struct NativeReplaySchedulerFrameInfo *info)
{
	fprintf(out, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%u,%u,%u,%u,%u,%u", info->frameTimer, info->frameCounter, info->timer, info->framesInThisLEV,
	        info->elapsedTimeMS, info->msInThisLEV, info->elapsedEventTime, info->mainGameState, info->loadingStage, info->levelID, info->mixRandomNumber,
	        info->audioRNG, info->deadcoed0, info->deadcoed1, info->advRng0, info->advRng1);
}

void ReferenceDump_ExportSessionFromReplay(const char *replayPath, const char *sessionPath, const char *frameInfoPath)
{
	FILE *in = fopen(replayPath, "rb");
	FILE *sess = (in != NULL && sessionPath != NULL) ? fopen(sessionPath, "w") : NULL;
	FILE *info = (in != NULL && frameInfoPath != NULL) ? fopen(frameInfoPath, "w") : NULL;
	struct NativeReplayFileHeader header;
	struct NativeReplayFrameRecord record;
	unsigned int i;
	int slot;

	if (in == NULL || (sess == NULL && info == NULL))
	{
		if (in != NULL)
		{
			fclose(in);
		}
		if (sess != NULL)
		{
			fclose(sess);
		}
		if (info != NULL)
		{
			fclose(info);
		}
		fprintf(stderr, "[CTR Reference] export-replay: cannot open %s (session=%s frameinfo=%s)\n", replayPath, sessionPath != NULL ? sessionPath : "-",
		        frameInfoPath != NULL ? frameInfoPath : "-");
		return;
	}

	if (fread(&header, sizeof(header), 1, in) != 1)
	{
		fclose(in);
		if (sess != NULL)
		{
			fclose(sess);
		}
		if (info != NULL)
		{
			fclose(info);
		}
		return;
	}

	for (i = 0; i < header.frameCount; i++)
	{
		if (fread(&record, sizeof(record), 1, in) != 1)
		{
			break;
		}

		if (i == 0)
		{
			if (sess != NULL)
			{
				fprintf(sess, "CTRSESSION,version=2,rngSeed=%u,gamepadSlots=8,frameCount=%u,raceStartFrame=\n", record.beginInfo.mixRandomNumber,
				        header.frameCount);
				fprintf(sess, "frame,dtMs");
				for (slot = 0; slot < 8; slot++)
				{
					fprintf(sess, ",p%d_buttons,p%d_steer", slot, slot);
				}
				fprintf(sess, "\n");
			}
			if (info != NULL)
			{
				fprintf(info, "frame,b_frameTimer,b_frameCounter,b_timer,b_framesInThisLEV,b_elapsedTimeMS,b_msInThisLEV,"
				              "b_elapsedEventTime,b_mainGameState,b_loadingStage,b_levelID,b_mixRNG,b_audioRNG,b_deadcoed0,"
				              "b_deadcoed1,b_advRng0,b_advRng1,e_frameTimer,e_frameCounter,e_timer,e_framesInThisLEV,"
				              "e_elapsedTimeMS,e_msInThisLEV,e_elapsedEventTime,e_mainGameState,e_loadingStage,e_levelID,"
				              "e_mixRNG,e_audioRNG,e_deadcoed0,e_deadcoed1,e_advRng0,e_advRng1\n");
			}
		}

		if (sess != NULL)
		{
			fprintf(sess, "%u,%d", record.replayFrame, record.endInfo.elapsedTimeMS);
			for (slot = 0; slot < 8; slot++)
			{
				int buttons = 0;
				int steer = 0;
				if (slot < PLATFORM_INPUT_PAD_COUNT && record.pads[slot].connected != 0)
				{
					unsigned int psxLow = (unsigned int)(record.pads[slot].buttons[0] | (record.pads[slot].buttons[1] << 8));
					buttons = ReferenceDump_MapPsxButtons((~psxLow) & 0xFFFF);
					steer = ReferenceDump_MapSteer(record.pads[slot].analog[2]);
				}
				fprintf(sess, ",%d,%d", buttons, steer);
			}
			fprintf(sess, "\n");
		}

		if (info != NULL)
		{
			fprintf(info, "%u,", record.replayFrame);
			ReferenceDump_WriteFrameInfoFields(info, &record.beginInfo);
			fprintf(info, ",");
			ReferenceDump_WriteFrameInfoFields(info, &record.endInfo);
			fprintf(info, "\n");
		}
	}

	fclose(in);
	if (sess != NULL)
	{
		fclose(sess);
	}
	if (info != NULL)
	{
		fclose(info);
	}
	fprintf(stderr, "[CTR Reference] export-replay: wrote %u frames (session=%s frameinfo=%s)\n", i, sessionPath != NULL ? sessionPath : "-",
	        frameInfoPath != NULL ? frameInfoPath : "-");
}

int ReferenceDump_MaybeExportSession(int argc, char **argv)
{
	const char *replayPath = ReferenceDump_ArgValue(argc, argv, "--export-replay");
	const char *schemaPath = ReferenceDump_ArgValue(argc, argv, "--dump-schema");
	int handled = 0;

	if (schemaPath != NULL)
	{
		ReferenceDump_WriteSchema(schemaPath);
		fprintf(stderr, "[CTR Reference] wrote full.bin decode schema to %s\n", schemaPath);
		handled = 1;
	}

	if (replayPath != NULL)
	{
		const char *sessionPath = ReferenceDump_ArgValue(argc, argv, "--dump-session");
		const char *frameInfoPath = ReferenceDump_ArgValue(argc, argv, "--dump-frameinfo");
		if (sessionPath == NULL && frameInfoPath == NULL)
		{
			sessionPath = "session.csv";
		}
		ReferenceDump_ExportSessionFromReplay(replayPath, sessionPath, frameInfoPath);
		handled = 1;
	}

	return handled;
}

void ReferenceDump_ConfigureFromArgs(int argc, char **argv)
{
	const char *statePath = ReferenceDump_ArgValue(argc, argv, "--dump-state");
	const char *fullPath = ReferenceDump_ArgValue(argc, argv, "--dump-full");
	const char *sessionPath = ReferenceDump_ArgValue(argc, argv, "--dump-session");
	const char *itemsPath = ReferenceDump_ArgValue(argc, argv, "--dump-items");
	const char *visibilityPath = ReferenceDump_ArgValue(argc, argv, "--dump-visibility");

	if (statePath != NULL)
	{
		s_referenceStateFile = fopen(statePath, "w");
		if (s_referenceStateFile != NULL)
		{
			fprintf(s_referenceStateFile, "tick,dtMs,rng,kart,posX,posY,posZ,velX,velY,velZ,angle,velYaw,velPitch,speed,state,lap\n");
		}
	}

	if (fullPath != NULL)
	{
		char schemaPath[512];
		s_referenceFullFile = fopen(fullPath, "wb");
		if (s_referenceFullFile != NULL)
		{
			unsigned int version = 1;
			unsigned int sizeofDriver = (unsigned int)sizeof(struct Driver);
			unsigned int sizeofGameTracker = (unsigned int)sizeof(struct GameTracker);
			unsigned int slots = 8;
			fwrite("CTRDUMP1", 1, 8, s_referenceFullFile);
			fwrite(&version, sizeof(version), 1, s_referenceFullFile);
			fwrite(&sizeofDriver, sizeof(sizeofDriver), 1, s_referenceFullFile);
			fwrite(&sizeofGameTracker, sizeof(sizeofGameTracker), 1, s_referenceFullFile);
			fwrite(&slots, sizeof(slots), 1, s_referenceFullFile);
		}
		snprintf(schemaPath, sizeof(schemaPath), "%s.schema.tsv", fullPath);
		ReferenceDump_WriteSchema(schemaPath);
	}

	if (sessionPath != NULL)
	{
		s_referenceSessionFile = fopen(sessionPath, "w");
	}

	if (itemsPath != NULL)
	{
		s_referenceItemsFile = fopen(itemsPath, "w");
		if (s_referenceItemsFile != NULL)
		{
			fprintf(s_referenceItemsFile, "tick,dtMs,rng,itemIdx,kind,modelID,posX,posY,posZ,velX,velY,velZ,ownerID,targetID,flags,timer,aux,rotY\n");
		}
	}

	if (visibilityPath != NULL)
	{
		s_referenceVisibilityFile = fopen(visibilityPath, "w");
		if (s_referenceVisibilityFile != NULL)
		{
			fprintf(s_referenceVisibilityFile, "tick,dtMs,rng,player,camCellQuad,quadHit,visQuadCount,visLeafCount,bspLeafsDrawn,numQuad\n");
		}
	}
}

static void ReferenceDump_WriteItemRow(int tick, int dtMs, int rng, int *itemIdx, const struct Thread *t, int kind)
{
	const struct Instance *inst = t->inst;
	int model = (inst != NULL && inst->model != NULL) ? (int)inst->model->id : -1;
	int px = 0, py = 0, pz = 0, vx = 0, vy = 0, vz = 0, owner = -1, target = -1, flags = 0, timer = 0, aux = 0, rotY = 0;

	if (inst != NULL)
	{
		px = inst->matrix.t[0];
		py = inst->matrix.t[1];
		pz = inst->matrix.t[2];
	}
	if (kind == 0)
	{
		const struct MineWeapon *mw = (const struct MineWeapon *)t->object;
		vx = mw->velocity.x;
		vy = mw->velocity.y;
		vz = mw->velocity.z;
		owner = (mw->instParent != NULL) ? (int)((struct Driver *)mw->instParent->thread->object)->driverID : -1;
		target = (mw->driverTarget != NULL) ? (int)mw->driverTarget->driverID : -1;
		flags = (int)mw->flags;
		timer = (int)mw->cooldown;
		aux = (int)mw->boolDestroyed;
	}
	else
	{
		const struct TrackerWeapon *tw = (const struct TrackerWeapon *)t->object;
		vx = tw->vel.x;
		vy = tw->vel.y;
		vz = tw->vel.z;
		owner = (tw->driverParent != NULL) ? (int)tw->driverParent->driverID : -1;
		target = (tw->driverTarget != NULL) ? (int)tw->driverTarget->driverID : -1;
		flags = (int)tw->flags;
		timer = tw->timeAlive;
		aux = (int)tw->parentSafetyFrames;
		rotY = (int)tw->rotY;
	}
	fprintf(s_referenceItemsFile, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", tick, dtMs, rng, (*itemIdx)++, kind, model, px, py, pz, vx, vy, vz,
	        owner, target, flags, timer, aux, rotY);
}

static void ReferenceDump_WriteItems(const struct GameTracker *gGT, int tick, int dtMs, int rng)
{
	const struct Thread *t;
	int itemIdx = 0;
	int i;

	for (t = gGT->threadBuckets[MINE].thread; t != NULL; t = t->siblingThread)
	{
		if ((t->flags & THREAD_FLAG_DEAD) == 0 && t->inst != NULL && t->object != NULL)
		{
			ReferenceDump_WriteItemRow(tick, dtMs, rng, &itemIdx, t, 0);
		}
	}
	for (t = gGT->threadBuckets[TRACKING].thread; t != NULL; t = t->siblingThread)
	{
		if ((t->flags & THREAD_FLAG_DEAD) == 0 && t->inst != NULL && t->object != NULL)
		{
			ReferenceDump_WriteItemRow(tick, dtMs, rng, &itemIdx, t, 1);
		}
	}
	for (i = 0; i < (int)gGT->numPlyrCurrGame; i++)
	{
		const struct Driver *d = gGT->drivers[i];
		if (d == NULL || d->instSelf == NULL || d->instSelf->thread == NULL)
		{
			continue;
		}
		for (t = d->instSelf->thread->childThread; t != NULL; t = t->siblingThread)
		{
			if ((t->flags & THREAD_FLAG_DEAD) != 0 || t->inst == NULL || t->object == NULL || t->inst->model == NULL)
			{
				continue;
			}
			if (t->inst->model->id == DYNAMIC_BOMB)
			{
				ReferenceDump_WriteItemRow(tick, dtMs, rng, &itemIdx, t, 1);
			}
		}
	}
	fflush(s_referenceItemsFile);
}

static int ReferenceDump_PopcountWords(const int *words, int bitCount)
{
	int total = 0;
	int wordCount = (bitCount + 31) >> 5;
	int w;
	if (words == NULL)
	{
		return 0;
	}
	for (w = 0; w < wordCount; w++)
	{
		unsigned int v = (unsigned int)words[w];
		while (v != 0)
		{
			total += (int)(v & 1u);
			v >>= 1;
		}
	}
	return total;
}

static void ReferenceDump_WriteVisibility(const struct GameTracker *gGT, int tick, int dtMs, int rng)
{
	const struct mesh_info *mesh = (gGT->level1 != NULL) ? gGT->level1->ptr_mesh_info : NULL;
	int p;

	if (mesh == NULL || gGT->visMem1 == NULL)
	{
		return;
	}
	for (p = 0; p < (int)gGT->numPlyrCurrGame; p++)
	{
		const struct CameraDC *cam = &gGT->cameraDC[p];
		int camCell = (cam->ptrQuadBlock != NULL) ? (int)(cam->ptrQuadBlock - mesh->ptrQuadBlockArray) : -1;
		int visQuad = ReferenceDump_PopcountWords(gGT->visMem1->visFaceList[p], mesh->numQuadBlock);
		int visLeaf = ReferenceDump_PopcountWords(gGT->visMem1->visLeafList[p], mesh->numBspNodes);
		fprintf(s_referenceVisibilityFile, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", tick, dtMs, rng, p, camCell, (int)cam->quadBlockSearchHit, visQuad, visLeaf,
		        gGT->bspLeafsDrawn, mesh->numQuadBlock);
	}
	fflush(s_referenceVisibilityFile);
}

void ReferenceDump_Tick(const struct GameTracker *gGT)
{
	unsigned int frame;
	int tick;
	int rng;
	int dtMs;
	int i;

	if ((s_referenceStateFile == NULL && s_referenceFullFile == NULL && s_referenceSessionFile == NULL && s_referenceItemsFile == NULL &&
	     s_referenceVisibilityFile == NULL) ||
	    gGT == NULL)
	{
		return;
	}

	frame = NativeReplayScheduler_ReplayFrame();
	rng = sdata->randomNumber;
	dtMs = gGT->elapsedTimeMS;

	if (s_referenceSessionFile != NULL)
	{
		ReferenceDump_WriteSessionRow(frame, dtMs, rng);
	}

	if (gGT->numPlyrCurrGame == 0 || gGT->drivers[0] == NULL)
	{
		return;
	}

	if (s_referenceRaceStarted == 0)
	{
		s_referenceRaceStarted = 1;
		s_referenceRaceStartFrame = frame;
		if (s_referenceStateFile != NULL)
		{
			fprintf(s_referenceStateFile, "# raceStartFrame=%u raceStartRng=%d\n", frame, rng);
		}
	}
	tick = (int)(frame - s_referenceRaceStartFrame);

	if (s_referenceItemsFile != NULL)
	{
		ReferenceDump_WriteItems(gGT, tick, dtMs, rng);
	}

	if (s_referenceVisibilityFile != NULL)
	{
		ReferenceDump_WriteVisibility(gGT, tick, dtMs, rng);
	}

	if (s_referenceStateFile != NULL)
	{
		for (i = 0; i < (int)gGT->numPlyrCurrGame; i++)
		{
			const struct Driver *d = gGT->drivers[i];
			if (d == NULL)
			{
				continue;
			}
			fprintf(s_referenceStateFile, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", tick, dtMs, rng, (int)d->driverID, d->posCurr.x, d->posCurr.y,
			        d->posCurr.z, d->velocity.x, d->velocity.y, d->velocity.z, (int)d->angle, (int)d->axisRotationX, (int)d->axisRotationY, (int)d->speed,
			        (int)d->kartState, (int)d->lapIndex);
		}
		fflush(s_referenceStateFile);
	}

	if (s_referenceFullFile != NULL)
	{
		unsigned int rngU = (unsigned int)rng;
		unsigned char mask = 0;
		int n = (int)gGT->numPlyrCurrGame;
		if (n > 8)
		{
			n = 8;
		}
		for (i = 0; i < n; i++)
		{
			if (gGT->drivers[i] != NULL)
			{
				mask |= (unsigned char)(1u << i);
			}
		}
		fwrite(&tick, sizeof(tick), 1, s_referenceFullFile);
		fwrite(&rngU, sizeof(rngU), 1, s_referenceFullFile);
		fwrite(&dtMs, sizeof(dtMs), 1, s_referenceFullFile);
		fwrite(&mask, 1, 1, s_referenceFullFile);
		for (i = 0; i < n; i++)
		{
			if (gGT->drivers[i] != NULL)
			{
				fwrite(gGT->drivers[i], sizeof(struct Driver), 1, s_referenceFullFile);
			}
		}
		fwrite(gGT, sizeof(struct GameTracker), 1, s_referenceFullFile);
		fflush(s_referenceFullFile);
	}
}

#endif /* CTR_REFERENCE */
