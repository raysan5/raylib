/*******************************************************************************************
*
*   raylib example - magicavoxel loading
*
*   This example has been created using raylib 3.8 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2021 Johann Nadalutti
*
********************************************************************************************/
#include "raylib.h"
#include "raymath.h"

#include <string.h>

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif

// Functions declarations
Model Vox_LoadFileName(const char* pszfileName);

// VOX Files to load

#define NUM_VOX_FILES  3

char* szVoxFiles[] = {
	"..\\..\\examples\\models\\resources\\vox\\chr_knight.vox",
	"..\\..\\examples\\models\\resources\\vox\\chr_sword.vox",
	"..\\..\\examples\\models\\resources\\vox\\monu9.vox"
};


int main(void)
{
	// Initialization
	//--------------------------------------------------------------------------------------
	const int screenWidth = 800;
	const int screenHeight = 450;

	InitWindow(screenWidth, screenHeight, "raylib [models] example - magicavoxel loading");

	// Load MagicaVoxel files
	Model models[NUM_VOX_FILES] = { 0 };

	for (int i = 0; i < NUM_VOX_FILES; i++)
	{
		// Load MagicaVoxel File and build vertices/colors/indices arrays
		double t0, t1;
		t0 = GetTime() * 1000.0;
		models[i] = Vox_LoadFileName(szVoxFiles[i]);
		t1 = GetTime() * 1000.0;
		TraceLog(LOG_INFO, TextFormat("Vox <%s> loaded in %f ms", GetFileName(szVoxFiles[i]), t1 - t0));


		//Model center
		BoundingBox  bb = GetModelBoundingBox(models[i]);
		Vector3 center;
		center.x = -(((bb.max.x - bb.min.x) / 2));
		center.y = -(((bb.max.y - bb.min.y) / 2));
		center.z = -(((bb.max.z - bb.min.z) / 2));

		Matrix matP = MatrixTranslate(center.x, center.z, 0);
		Matrix matR = MatrixRotateX(90 * DEG2RAD);
		models[i].transform = MatrixMultiply(matP, matR);
	}


	// Define the camera to look into our 3d world
	Camera camera = { { 0.0f, 10.0f, 10.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f, 0 };

	// Model drawing position
	Vector3 position = { 0.0f, 0.0f, 0.0f };

	int currentModel = 0;



	SetCameraMode(camera, CAMERA_ORBITAL);  // Set a orbital camera mode

	SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

	//--------------------------------------------------------------------------------------
	// Main game loop
	//--------------------------------------------------------------------------------------
	while (!WindowShouldClose())    // Detect window close button or ESC key
	{
		//--------------------------------------------------------------------------------------
		// Update
		//----------------------------------------------------------------------------------
		UpdateCamera(&camera);      // Update internal camera and our camera

		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			currentModel = (currentModel + 1) % NUM_VOX_FILES; // Cycle between models
		}

		if (IsKeyPressed(KEY_RIGHT))
		{
			currentModel++;
			if (currentModel >= NUM_VOX_FILES) currentModel = 0;
		}
		else if (IsKeyPressed(KEY_LEFT))
		{
			currentModel--;
			if (currentModel < 0) currentModel = NUM_VOX_FILES - 1;
		}
		else if (IsKeyPressed(KEY_DOWN))
		{
			Matrix mat = MatrixTranslate(0, 0, 0.5);
			models[currentModel].transform = MatrixMultiply(models[currentModel].transform, mat);

		}
		else if (IsKeyPressed(KEY_UP))
		{
			Matrix mat = MatrixTranslate(0, 0, -0.5);
			models[currentModel].transform = MatrixMultiply(models[currentModel].transform, mat);

		}

		//----------------------------------------------------------------------------------
		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();

		ClearBackground(RAYWHITE);

		//Display model
		BeginMode3D(camera);

		Vector3 rotAxis = { 1,0,0 };
		Vector3 scale = { 1,1,1 };


		DrawModelEx(models[currentModel], position, rotAxis, 0, scale, WHITE);
		//DrawModelWiresEx(models[currentModel], position, rotAxis, -90.0f, scale, BLACK);

		DrawGrid(10, 1.0);

		EndMode3D();

		//Display debug infos
		DrawRectangle(30, 400, 310, 30, Fade(SKYBLUE, 0.5f));
		DrawRectangleLines(30, 400, 310, 30, Fade(DARKBLUE, 0.5f));
		DrawText("MOUSE LEFT BUTTON to CYCLE VOX MODELS", 40, 410, 10, BLUE);

		DrawText(GetFileName(szVoxFiles[currentModel]), 100, 10, 20, DARKBLUE);

		EndDrawing();
		//----------------------------------------------------------------------------------
	}

	// De-Initialization
	//--------------------------------------------------------------------------------------

	// Unload models data (GPU VRAM)
	for (int i = 0; i < NUM_VOX_FILES; i++) UnloadModel(models[i]);

	CloseWindow();          // Close window and OpenGL context
	//--------------------------------------------------------------------------------------

	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// ArrayInt
/////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int* array;
	int used;
	int size;
} ArrayInt;

void initArrayInt(ArrayInt* a, int initialSize)
{
	a->array = MemAlloc(initialSize * sizeof(int));
	a->used = 0;
	a->size = initialSize;
}

void insertArrayInt(ArrayInt* a, int element)
{
	if (a->used == a->size)
	{
		a->size *= 2;
		a->array = MemRealloc(a->array, a->size * sizeof(int));
	}
	a->array[a->used++] = element;
}

void freeArrayInt(ArrayInt* a)
{
	MemFree(a->array);
	a->array = NULL;
	a->used = a->size = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// ArrayUShort
/////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	unsigned short* array;
	int used;
	int size;
} ArrayUShort;

void initArrayUShort(ArrayUShort* a, int initialSize)
{
	a->array = MemAlloc(initialSize * sizeof(unsigned short));
	a->used = 0;
	a->size = initialSize;
}

void insertArrayUShort(ArrayUShort* a, unsigned short element)
{
	if (a->used == a->size)
	{
		a->size *= 2;
		a->array = MemRealloc(a->array, a->size * sizeof(unsigned short));
	}
	a->array[a->used++] = element;
}

void freeArrayUShort(ArrayUShort* a)
{
	MemFree(a->array);
	a->array = NULL;
	a->used = a->size = 0;
}



/////////////////////////////////////////////////////////////////////////////////////////////
// ArrayFloat
/////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	float* array;
	int used;
	int size;
} ArrayFloat;

void initArrayFloat(ArrayFloat* a, int initialSize)
{
	a->array = MemAlloc(initialSize * sizeof(float));
	a->used = 0;
	a->size = initialSize;
}

void insertArrayFloat(ArrayFloat* a, float element)
{
	if (a->used == a->size)
	{
		a->size *= 2;
		a->array = MemRealloc(a->array, a->size * sizeof(float));
	}
	a->array[a->used++] = element;
}

void freeArrayFloat(ArrayFloat* a)
{
	MemFree(a->array);
	a->array = NULL;
	a->used = a->size = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// ArrayVector3
/////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	Vector3* array;
	int used;
	int size;
} ArrayVector3;

void initArrayVector3(ArrayVector3* a, int initialSize)
{
	a->array = MemAlloc(initialSize * sizeof(Vector3));
	a->used = 0;
	a->size = initialSize;
}

void insertArrayVector3(ArrayVector3* a, Vector3 element)
{
	if (a->used == a->size)
	{
		a->size *= 2;
		a->array = MemRealloc(a->array, a->size * sizeof(Vector3));
	}
	a->array[a->used++] = element;
}

void freeArrayVector3(ArrayVector3* a)
{
	MemFree(a->array);
	a->array = NULL;
	a->used = a->size = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// ArrayColor
/////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	Color* array;
	int used;
	int size;
} ArrayColor;

void initArrayColor(ArrayColor* a, int initialSize)
{
	a->array = MemAlloc(initialSize * sizeof(Color));
	a->used = 0;
	a->size = initialSize;
}

void insertArrayColor(ArrayColor* a, Color element)
{
	if (a->used == a->size)
	{
		a->size *= 2;
		a->array = MemRealloc(a->array, a->size * sizeof(Color));
	}
	a->array[a->used++] = element;
}

void freeArrayColor(ArrayColor* a)
{
	MemFree(a->array);
	a->array = NULL;
	a->used = a->size = 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Vox
/////////////////////////////////////////////////////////////////////////////////////////////

#define CHUNKSIZE 16 // chunk size (CHUNKSIZE*CHUNKSIZE*CHUNKSIZE) in voxels 
#define CHUNKSIZE_OPSHIFT 4 // 1<<4=16 -> depend of CHUNKSIZE
#define CHUNK_FLATTENOFFSET_OPSHIFT 8

//
// used right handed system and CCW face
// 
// indexes for voxelcoords, per face orientation
//

//#      Y
//#      |
//#      o----X
//#     /
//#    Z     2------------3
//#         /|           /|
//#        6------------7 |
//#        | |          | |
//#        |0 ----------|- 1
//#        |/           |/
//#        4------------5

// 
// CCW
const int fv[6][4] = {
	{0, 2, 6, 4 }, //-X
	{5, 7, 3, 1 }, //+X
	{0, 4, 5, 1 }, //-y
	{6, 2, 3, 7 }, //+y
	{1, 3, 2, 0 }, //-Z
	{4, 6, 7, 5 } };//+Z


const Vector3 SolidVertex[] = {
	{0, 0, 0},   //0
	{1, 0, 0},   //1
	{0, 1, 0},   //2
	{1, 1, 0},   //3
	{0, 0, 1},   //4
	{1, 0, 1},   //5
	{0, 1, 1},   //6
	{1, 1, 1} }; //7


// A chunk that contain voxels
typedef struct VoxChunk3D
{
	unsigned char* m_array; //If Sparse != null
	int arraySize; //Size for m_array in bytes (DEBUG ONLY)
} CubeChunk3D;

// Array for voxels
// Array is divised into chunks of CHUNKSIZE*CHUNKSIZE*CHUNKSIZE voxels size
typedef struct VoxArray3D
{
	//Array size in voxels
	int sizeX;
	int sizeY;
	int sizeZ;

	//Chunks size into array (array is divised into chunks)
	int chunksSizeX;
	int chunksSizeY;
	int chunksSizeZ;

	//Chunks array
	CubeChunk3D* m_arrayChunks;
	int arrayChunksSize; //Size for m_arrayChunks in bytes (DEBUG ONLY)

	int ChunkFlattenOffset;
	int chunksAllocated;
	int chunksTotal;

	//Arrays for mesh build
	ArrayVector3 vertices;
	ArrayUShort indices;
	ArrayColor colors;

	//Palette for voxels
	Color palette[256];

} VoxArray3D;



// Allocated VoxArray3D size
VoxArray3D* Vox_AllocArray(int _sx, int _sy, int _sz)
{
	int sx = _sx + ((CHUNKSIZE - (_sx % CHUNKSIZE)) % CHUNKSIZE);
	int sy = _sy + ((CHUNKSIZE - (_sy % CHUNKSIZE)) % CHUNKSIZE);
	int sz = _sz + ((CHUNKSIZE - (_sz % CHUNKSIZE)) % CHUNKSIZE);

	int chx = sx >> CHUNKSIZE_OPSHIFT; //Chunks Count in X
	int chy = sy >> CHUNKSIZE_OPSHIFT; //Chunks Count in Y
	int chz = sz >> CHUNKSIZE_OPSHIFT; //Chunks Count in Z

	VoxArray3D* parray = (VoxArray3D*)MemAlloc(sizeof(VoxArray3D));
	parray->sizeX = sx;
	parray->sizeY = sy;
	parray->sizeZ = sz;

	parray->chunksSizeX = chx;
	parray->chunksSizeY = chy;
	parray->chunksSizeZ = chz;

	parray->ChunkFlattenOffset = (chy * chz); //m_arrayChunks[(x * (sy*sz)) + (z * sy) + y]

	//Alloc chunks array
	int size = sizeof(CubeChunk3D) * chx * chy * chz;
	parray->m_arrayChunks = MemAlloc(size);
	parray->arrayChunksSize = size;


	//Init chunks array
	size = chx * chy * chz;
	parray->chunksTotal = size;
	parray->chunksAllocated = 0;

	for (int i = 0; i < size; i++)
	{
		parray->m_arrayChunks[i].m_array = 0;
		parray->m_arrayChunks[i].arraySize = 0;
	}

	return parray;
}

// Set voxel ID from its position into VoxArray3D
void Vox_SetVoxel(VoxArray3D* _parray, int x, int y, int z, unsigned char id)
{
	//Get chunk from array pos
	int chX = x >> CHUNKSIZE_OPSHIFT; //x / CHUNKSIZE;
	int chY = y >> CHUNKSIZE_OPSHIFT; //y / CHUNKSIZE;
	int chZ = z >> CHUNKSIZE_OPSHIFT; //z / CHUNKSIZE;
	int offset = (chX * _parray->ChunkFlattenOffset) + (chZ * _parray->chunksSizeY) + chY;

	if (offset > _parray->arrayChunksSize)
	{
		TraceLog(LOG_ERROR, "Out of array");
	}

	CubeChunk3D* chunk = &_parray->m_arrayChunks[offset];

	//Set Chunk
	chX = x - (chX << CHUNKSIZE_OPSHIFT); //x - (bx * CHUNKSIZE);
	chY = y - (chY << CHUNKSIZE_OPSHIFT); //y - (by * CHUNKSIZE);
	chZ = z - (chZ << CHUNKSIZE_OPSHIFT); //z - (bz * CHUNKSIZE);

	if (chunk->m_array == 0)
	{
		int size = CHUNKSIZE * CHUNKSIZE * CHUNKSIZE;
		chunk->m_array = MemAlloc(size);
		chunk->arraySize = size;
		//memset(chunk->m_array, 0, size);

		_parray->chunksAllocated++;
	}

	offset = (chX << CHUNK_FLATTENOFFSET_OPSHIFT) + (chZ << CHUNKSIZE_OPSHIFT) + chY;

	if (offset > chunk->arraySize)
	{
		TraceLog(LOG_ERROR, "Out of array");
	}

	chunk->m_array[offset] = id;

}

// Get voxel ID from its position into VoxArray3D
unsigned char Vox_GetVoxel(VoxArray3D* _parray, int x, int y, int z)
{
	if (x < 0 || y < 0 || z < 0)
		return 0;

	if (x >= _parray->sizeX || y >= _parray->sizeY || z >= _parray->sizeZ)
		return 0;


	//Get chunk from array pos
	int chX = x >> CHUNKSIZE_OPSHIFT; //x / CHUNKSIZE;
	int chY = y >> CHUNKSIZE_OPSHIFT; //y / CHUNKSIZE;
	int chZ = z >> CHUNKSIZE_OPSHIFT; //z / CHUNKSIZE;
	int offset = (chX * _parray->ChunkFlattenOffset) + (chZ * _parray->chunksSizeY) + chY;

	if (offset > _parray->arrayChunksSize)
	{
		TraceLog(LOG_ERROR, "Out of array");
	}

	CubeChunk3D* chunk = &_parray->m_arrayChunks[offset];

	//Set Chunk
	chX = x - (chX << CHUNKSIZE_OPSHIFT); //x - (bx * CHUNKSIZE);
	chY = y - (chY << CHUNKSIZE_OPSHIFT); //y - (by * CHUNKSIZE);
	chZ = z - (chZ << CHUNKSIZE_OPSHIFT); //z - (bz * CHUNKSIZE);

	if (chunk->m_array == 0)
	{
		return 0;
	}

	offset = (chX << CHUNK_FLATTENOFFSET_OPSHIFT) + (chZ << CHUNKSIZE_OPSHIFT) + chY;

	if (offset > chunk->arraySize)
	{
		TraceLog(LOG_ERROR, "Out of array");
	}
	return chunk->m_array[offset];

}

// Calc visibles faces from a voxel position
unsigned char Vox_CalcFacesVisible(VoxArray3D* pvoxArray, int cx, int cy, int cz)
{
	unsigned char idXm1 = Vox_GetVoxel(pvoxArray, cx - 1, cy, cz);
	unsigned char idXp1 = Vox_GetVoxel(pvoxArray, cx + 1, cy, cz);

	unsigned char idYm1 = Vox_GetVoxel(pvoxArray, cx, cy - 1, cz);
	unsigned char idYp1 = Vox_GetVoxel(pvoxArray, cx, cy + 1, cz);

	unsigned char idZm1 = Vox_GetVoxel(pvoxArray, cx, cy, cz - 1);
	unsigned char idZp1 = Vox_GetVoxel(pvoxArray, cx, cy, cz + 1);

	unsigned char byVFMask = 0;

	//#-x
	if (idXm1 == 0)
		byVFMask |= (1 << 0);

	//#+x
	if (idXp1 == 0)
		byVFMask |= (1 << 1);

	//#-y
	if (idYm1 == 0)
		byVFMask |= (1 << 2);

	//#+y
	if (idYp1 == 0)
		byVFMask |= (1 << 3);

	//#-z
	if (idZm1 == 0)
		byVFMask |= (1 << 4);

	//#+z
	if (idZp1 == 0)
		byVFMask |= (1 << 5);

	return byVFMask;
}

// Get a vertex position from a voxel's corner
Vector3 Vox_GetVertexPosition(int _wcx, int _wcy, int _wcz, int _nNumVertex)
{
	float scale = 0.25;
	Vector3 vtx = SolidVertex[_nNumVertex];
	vtx.x = (vtx.x + _wcx) * scale;
	vtx.y = (vtx.y + _wcy) * scale;
	vtx.z = (vtx.z + _wcz) * scale;
	return vtx;
}

// Build a voxel vertices/colors/indices
void Vox_Build_Voxel(VoxArray3D* pvoxArray, int x, int y, int z, int matID)
{

	unsigned char byVFMask = Vox_CalcFacesVisible(pvoxArray, x, y, z);

	if (byVFMask == 0)
		return;

	int i, j;
	Vector3 vertComputed[8];
	int bVertexComputed[8];
	memset(vertComputed, 0, sizeof(vertComputed));
	memset(bVertexComputed, 0, sizeof(bVertexComputed));


	//For each Cube's faces
	for (i = 0; i < 6; i++) // 6 faces
	{
		if ((byVFMask & (1 << i)) != 0)	//If face is visible
		{
			for (j = 0; j < 4; j++)   // 4 corners
			{
				int  nNumVertex = fv[i][j];  //Face,Corner
				if (bVertexComputed[nNumVertex] == 0) //if never calc
				{
					bVertexComputed[nNumVertex] = 1;
					vertComputed[nNumVertex] = Vox_GetVertexPosition(x, y, z, nNumVertex);
				}
			}
		}
	}

	//Add face
	for (i = 0; i < 6; i++)// 6 faces
	{
		if ((byVFMask & (1 << i)) == 0)
			continue; //Face invisible

		int v0 = fv[i][0];  //Face, Corner
		int v1 = fv[i][1];  //Face, Corner
		int v2 = fv[i][2];  //Face, Corner
		int v3 = fv[i][3];  //Face, Corner

		//Arrays
		int idx = pvoxArray->vertices.used;
		insertArrayVector3(&pvoxArray->vertices, vertComputed[v0]);
		insertArrayVector3(&pvoxArray->vertices, vertComputed[v1]);
		insertArrayVector3(&pvoxArray->vertices, vertComputed[v2]);
		insertArrayVector3(&pvoxArray->vertices, vertComputed[v3]);

		Color col = pvoxArray->palette[matID];
		//col.r = 0;
		//col.g = 0;
		//col.b = 255;
		//col.a = 255;
		insertArrayColor(&pvoxArray->colors, col);
		insertArrayColor(&pvoxArray->colors, col);
		insertArrayColor(&pvoxArray->colors, col);
		insertArrayColor(&pvoxArray->colors, col);


		//v0 - v1 - v2, v0 - v2 - v3
		//insertArrayUShort(&pvoxArray->indices, idx + 0);
		//insertArrayUShort(&pvoxArray->indices, idx + 1);
		//insertArrayUShort(&pvoxArray->indices, idx + 2);

		//insertArrayUShort(&pvoxArray->indices, idx + 0);
		//insertArrayUShort(&pvoxArray->indices, idx + 2);
		//insertArrayUShort(&pvoxArray->indices, idx + 3);

		insertArrayUShort(&pvoxArray->indices, idx + 0);
		insertArrayUShort(&pvoxArray->indices, idx + 2);
		insertArrayUShort(&pvoxArray->indices, idx + 1);

		insertArrayUShort(&pvoxArray->indices, idx + 0);
		insertArrayUShort(&pvoxArray->indices, idx + 3);
		insertArrayUShort(&pvoxArray->indices, idx + 2);



	}

}

// MagicaVoxel *.vox file format Loader and model builder
Model Vox_LoadFileName(const char* pszfileName)
{
	TraceLog(LOG_INFO, TextFormat("Loading VOX: %s", pszfileName));


	Model model = { 0 };

	VoxArray3D* pvoxArray = 0;

	//////////////////////////////////////////////////
	//Read VOX file
	//4 bytes: magic number ('V' 'O' 'X' 'space' )
	//4 bytes: version number (current version is 150 )

	unsigned long signature;

	unsigned long readed = 0;
	unsigned char* fileData;
	fileData = LoadFileData(pszfileName, &readed);
	if (fileData == 0)
	{
		return model;
	}

	unsigned char* fileDataPtr = fileData;
	unsigned char* endfileDataPtr = fileData + readed;

	signature = *((unsigned long*)fileDataPtr);
	fileDataPtr += sizeof(unsigned long);

	if (signature != 0x20584F56) //56 4F 58 20
	{
		TraceLog(LOG_ERROR, "Not an MagicaVoxel File format");
		return model;
	}

	unsigned long version;

	version = *((unsigned long*)fileDataPtr);
	fileDataPtr += sizeof(unsigned long);

	if (version < 150)
	{
		TraceLog(LOG_ERROR, "MagicaVoxel version too old");
		return model;
	}


	// header
	//4 bytes: chunk id
	//4 bytes: size of chunk contents (n)
	//4 bytes: total size of children chunks(m)

	//// chunk content
	//n bytes: chunk contents

	//// children chunks : m bytes
	//{ child chunk 0 }
	//{ child chunk 1 }
	unsigned long sizeX, sizeY, sizeZ;
	sizeX = sizeY = sizeZ = 0;
	unsigned long numVoxels = 0;
	int offsetX, offsetY, offsetZ;
	offsetX = offsetY = offsetZ = 0;

	while (fileDataPtr < endfileDataPtr)
	{
		char szChunkName[5];
		memcpy(szChunkName, fileDataPtr, 4);
		szChunkName[4] = 0;
		fileDataPtr += 4;

		unsigned long chunkSize = *((unsigned long*)fileDataPtr);
		fileDataPtr += sizeof(unsigned long);

		unsigned long chunkTotalChildSize = *((unsigned long*)fileDataPtr);
		fileDataPtr += sizeof(unsigned long);


		if (strcmp(szChunkName, "SIZE") == 0)
		{
			//(4 bytes x 3 : x, y, z ) 
			sizeX = *((unsigned long*)fileDataPtr);
			fileDataPtr += sizeof(unsigned long);

			sizeY = *((unsigned long*)fileDataPtr);
			fileDataPtr += sizeof(unsigned long);

			sizeZ = *((unsigned long*)fileDataPtr);
			fileDataPtr += sizeof(unsigned long);

			//Alloc vox array
			pvoxArray = Vox_AllocArray(sizeX, sizeY, sizeZ);
		}
		else if (strcmp(szChunkName, "XYZI") == 0)
		{
			unsigned char vx, vy, vz, vi;

			//(numVoxels : 4 bytes )
			//(each voxel: 1 byte x 4 : x, y, z, colorIndex ) x numVoxels
			numVoxels = *((unsigned long*)fileDataPtr);
			fileDataPtr += sizeof(unsigned long);

			while (numVoxels > 0)
			{
				vx = *((unsigned char*)fileDataPtr++);
				vy = *((unsigned char*)fileDataPtr++);
				vz = *((unsigned char*)fileDataPtr++);
				vi = *((unsigned char*)fileDataPtr++);

				Vox_SetVoxel(pvoxArray, vx, vy, vz, vi);

				numVoxels--;
			}
		}
		else if (strcmp(szChunkName, "RGBA") == 0)
		{
			unsigned char  r, g, b, a;

			//(each pixel: 1 byte x 4 : r, g, b, a ) x 256
			for (int i = 0; i < 256 - 1; i++)
			{
				Color col;
				col.r = *((unsigned char*)fileDataPtr++);
				col.g = *((unsigned char*)fileDataPtr++);
				col.b = *((unsigned char*)fileDataPtr++);
				col.a = *((unsigned char*)fileDataPtr++);

				pvoxArray->palette[i + 1] = col;
			}

		}
		else
		{
			fileDataPtr += chunkSize;
		}
	}

	TraceLog(LOG_INFO, TextFormat("Vox Size : %dx%dx%d", sizeX, sizeY, sizeZ));

	TraceLog(LOG_INFO, TextFormat("Vox Chunks Count : %d/%d", pvoxArray->chunksAllocated, pvoxArray->chunksTotal));


	//////////////////////////////////////////////////////////
	// Building Mesh
	//   TODO compute globals indices array

	TraceLog(LOG_INFO, TextFormat("Building VOX Mesh : %s", pszfileName));

	// Init Arrays
	initArrayVector3(&pvoxArray->vertices, 3 * 1024);
	initArrayUShort(&pvoxArray->indices, 3 * 1024);
	initArrayColor(&pvoxArray->colors, 3 * 1024);

	// Create vertices and indices buffers
	int x, y, z;

	for (x = 0; x <= pvoxArray->sizeX; x++)
	{
		for (z = 0; z <= pvoxArray->sizeZ; z++)
		{
			for (y = 0; y <= pvoxArray->sizeY; y++)
			{
				unsigned char matID = Vox_GetVoxel(pvoxArray, x, y, z);
				if (matID != 0)
					Vox_Build_Voxel(pvoxArray, x, y, z, matID);
			}
		}
	}


	//Compute meshes count
	int nbvertices = pvoxArray->vertices.used;
	int meshescount = 1 + (nbvertices / 65536);



	// Build Models from meshes
	model.transform = MatrixIdentity();

	model.meshCount = meshescount;
	model.meshes = (Mesh*)MemAlloc(model.meshCount * sizeof(Mesh));

	model.meshMaterial = (int*)MemAlloc(model.meshCount * sizeof(int));

	model.materialCount = 1;
	model.materials = (Material*)MemAlloc(model.materialCount * sizeof(Material));
	model.materials[0] = LoadMaterialDefault();


	// Init model's meshes
	int verticesRemain = pvoxArray->vertices.used;
	int verticesMax = 65532; //5461 voxels x 12 vertices per voxel -> 65532 (must be inf 65536)
	
	Vector3* pvertices = pvoxArray->vertices.array;	//6*4=12 vertices per voxel
	Color* pcolors = pvoxArray->colors.array;
	unsigned short* pindices = pvoxArray->indices.array;	//5461 * 6 * 6 -> 196596 indices max per mesh 

	int size;

	for (int idxMesh = 0; idxMesh < meshescount; idxMesh++)
	{
		Mesh* pmesh = &model.meshes[idxMesh];
		memset(pmesh, 0, sizeof(Mesh));

		// Copy Vertices
		pmesh->vertexCount = (int)fmin(verticesMax, verticesRemain);

		size = pmesh->vertexCount * sizeof(float) * 3;
		pmesh->vertices = MemAlloc(size);
		memcpy(pmesh->vertices, pvertices, size);

		//Copy Indices
		size = pvoxArray->indices.used * sizeof(unsigned short);
		pmesh->indices = MemAlloc(size);
		memcpy(pmesh->indices, pindices, size);

		pmesh->triangleCount = pvoxArray->indices.used / 3;

		// Copy Colors
		size = pmesh->vertexCount * sizeof(Color);
		pmesh->colors = MemAlloc(size);
		memcpy(pmesh->colors, pcolors, size);

		// First material index
		model.meshMaterial[idxMesh] = 0; 

		// Build GPU mesh
		UploadMesh(pmesh, false);

		//Next
		verticesRemain -= verticesMax;
		pvertices += verticesMax;
		pcolors += verticesMax;
	}

	//Free arrays
	freeArrayVector3(&pvoxArray->vertices);
	freeArrayUShort(&pvoxArray->indices);
	freeArrayColor(&pvoxArray->colors);


	return model;
}



