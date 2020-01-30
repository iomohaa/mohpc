#pragma once

#include <stdint.h>
#include <stddef.h>
#include <MOHPC/Math.h>

namespace MOHPC
{

#define	MAX_STRING_CHARS	2048	// max length of a string passed to Cmd_TokenizeString
#define	MAX_STRING_TOKENS	1024	// max tokens resulting from Cmd_TokenizeString
#define	MAX_TOKEN_CHARS		1024	// max length of an individual token

#define	MAX_INFO_STRING		1350
#define	MAX_INFO_KEY		  1024
#define	MAX_INFO_VALUE		1024

#define	BIG_INFO_STRING		8192  // used for system info key only
#define	BIG_INFO_KEY		8192
#define	BIG_INFO_VALUE		8192

#define	ANGLE2SHORT(x)	((int)((x)*65536/360) & 65535)
#define	BYTE2ANGLE(x)	((x)*(360.0f/255))
#define	SHORT2ANGLE(x)	((x)*(360.0f/65536))

#define NYT HMAX					/* NYT = Not Yet Transmitted */
#define INTERNAL_NODE (HMAX+1)

typedef struct nodetype {
	struct	nodetype *left, *right, *parent; /* tree structure */
	struct	nodetype *next, *prev; /* doubly-linked list */
	struct	nodetype **head; /* highest ranked node in block */
	int		weight;
	int		symbol;
} node_t;

#define HMAX 256 /* Maximum symbol */

typedef struct {
	int			blocNode;
	int			blocPtrs;

	node_t*		tree;
	node_t*		lhead;
	node_t*		ltail;
	node_t*		loc[HMAX + 1];
	node_t**	freelist;

	node_t		nodeList[768];
	node_t*		nodePtrs[768];
} huff_t;

typedef struct {
	huff_t		compressor;
	huff_t		decompressor;
} huffman_t;

typedef struct msg_s {
	bool	allowoverflow;	// if false, do a Com_Error
	bool	overflowed;		// set to true if the buffer size failed (with allowoverflow set)
	bool	oob;			// set to true if the buffer size failed (with allowoverflow set)
	uint8_t	*data;
	size_t	maxsize;
	size_t	cursize;
	size_t	readcount;
	int		bit;				// for bitwise reads and writes
} msg_t;

void	Huff_Compress(msg_t *buf, int offset);
void	Huff_Decompress(msg_t *buf, int offset);
void	Huff_Init(huffman_t *huff);
void	Huff_addRef(huff_t* huff, uint8_t ch);
int		Huff_Receive(node_t *node, int *ch, uint8_t *fin);
void	Huff_transmit(huff_t *huff, int ch, uint8_t *fout);
void	Huff_offsetReceive(node_t *node, int *ch, uint8_t *fin, int *offset);
void	Huff_offsetTransmit(huff_t *huff, int ch, uint8_t *fout, int *offset);
void	Huff_putBit(int bit, uint8_t *fout, int *offset);
int		Huff_getBit(uint8_t *fout, int *offset);


void MSG_Init(msg_t *buf, uint8_t *data, size_t length);
void MSG_InitOOB(msg_t *buf, uint8_t *data, size_t length);
void MSG_Clear(msg_t *buf);
void MSG_WriteData(msg_t *buf, const void *data, size_t length);
void MSG_Bitstream(msg_t *buf);

// TTimo
// copy a msg_t in case we need to store it as is for a bit
// (as I needed this to keep an msg_t from a static var for later use)
// sets data buffer as MSG_Init does prior to do the copy
void MSG_Copy(msg_t *buf, uint8_t *data, size_t length, msg_t *src);

struct usercmd_s;
struct entityState_s;
struct playerState_s;

void MSG_WriteBits(msg_t *msg, int value, int bits);

void MSG_WriteChar(msg_t *sb, int c);
void MSG_WriteByte(msg_t *sb, int c);
void MSG_WriteShort(msg_t *sb, int c);

void MSG_WriteSVC(msg_t *sb, int c);

void MSG_WriteLong(msg_t *sb, int c);
void MSG_WriteFloat(msg_t *sb, float f);
void MSG_WriteString(msg_t *sb, const char *s);
void MSG_WriteBigString(msg_t *sb, const char *s);
void MSG_WriteAngle(msg_t *sb, float f);
void MSG_WriteAngle16(msg_t *sb, float f);

void	MSG_BeginReading(msg_t *sb);
void	MSG_BeginReadingOOB(msg_t *sb);

int		MSG_ReadBits(msg_t *msg, int bits);

int		MSG_ReadChar(msg_t *sb);
int		MSG_ReadByte(msg_t *sb);
int		MSG_ReadSVC(msg_t *sb);
int		MSG_ReadShort(msg_t *sb);
int		MSG_ReadLong(msg_t *sb);

void MSG_ReadDir(msg_t *msg, vec3_t dir);
float MSG_ReadCoord(msg_t *msg);
//void MSG_GetNullEntityState(entityState_t *nullState);

float	MSG_ReadFloat(msg_t *sb);
char	*MSG_ReadString(msg_t *sb);
char	*MSG_ReadBigString(msg_t *sb);
char	*MSG_ReadStringLine(msg_t *sb);
float	MSG_ReadAngle8(msg_t *sb);
float	MSG_ReadAngle16(msg_t *sb);
void	MSG_ReadData(msg_t *sb, void *buffer, int size);

/*
void MSG_WriteDeltaUsercmd(msg_t *msg, struct usercmd_s *from, struct usercmd_s *to);
void MSG_ReadDeltaUsercmd(msg_t *msg, struct usercmd_s *from, struct usercmd_s *to);

void MSG_WriteDeltaUsercmdKey(msg_t *msg, int key, usercmd_t *from, usercmd_t *to);
void MSG_ReadDeltaUsercmdKey(msg_t *msg, int key, usercmd_t *from, usercmd_t *to);

void MSG_WriteDeltaEntity(msg_t *msg, struct entityState_s *from, struct entityState_s *to, qboolean force);

void MSG_ReadSounds(msg_t *msg, server_sound_t *sounds, int *snapshot_number_of_sounds);
void MSG_WriteSounds(msg_t *msg, server_sound_t *sounds, int snapshot_number_of_sounds);

void MSG_ReadDeltaEyeInfo(msg_t *msg, usereyes_t *from, usereyes_t *to);

void MSG_ReadDeltaEntity(msg_t *msg, entityState_t *from, entityState_t *to,
	int number);

void MSG_WriteDeltaEyeInfo(msg_t  *msg, usereyes_t *from, usereyes_t *to);

void MSG_WriteDeltaPlayerstate(msg_t *msg, struct playerState_s *from, struct playerState_s *to);
void MSG_ReadDeltaPlayerstate(msg_t *msg, struct playerState_s *from, struct playerState_s *to);
*/

};
