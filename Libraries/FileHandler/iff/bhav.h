/* BHAV chunk */

typedef struct TreeNodeParams
{
    uint16_t Param0;
    uint16_t Param1;
    uint16_t Param2;
    uint16_t Param3;
} IFF_TRETABLENODEPARAMS;

typedef struct TreeTableNode
{
    uint16_t PrimitiveNumber;
    uint8_t TransitionTrue;
    uint8_t TransitionFalse;
    IFF_TRETABLENODEPARAMS Parameters;
} IFF_TREETABLENODE;

typedef struct TreeTable
{
    uint16_t StreamVersion;
    uint8_t Type;
    uint8_t NumParams;
    uint8_t NumLocals;
    uint32_t TreeVersion;
    
    IFF_TREETABLENODE *NodesBegin;
    IFF_TREETABLENODE *NodesEnd;
} IFF_TREETABLE;