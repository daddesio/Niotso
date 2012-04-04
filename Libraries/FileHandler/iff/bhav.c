#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "iff.h"

int iff_parse_bhav(IFFChunk * ChunkInfo, const uint8_t * Buffer)
{
    IFF_TREETABLE * TreeTableData;
    if(ChunkInfo->Size < 12)
        return 0;
    ChunkInfo->FormattedData = malloc(17);
    if(ChunkInfo->FormattedData == NULL)
        return 0;
    memset(ChunkInfo->FormattedData, 0, 17);
    TreeTableData = (IFF_TREETABLE *)ChunkInfo->FormattedData;
    
    memcpy (TreeTableData, Buffer, 2);
    
    int nodeCount = 0;
    if (TreeTableData->StreamVersion == 0x8003)
    {
        memcpy (((char *)TreeTableData)+2, Buffer+2, 7);
        memcpy (&nodeCount, Buffer+9, 4);
    }
    else if ((TreeTableData->StreamVersion == 0x8000) || (TreeTableData->StreamVersion == 0x8001) || (TreeTableData->StreamVersion == 0x8002))
    {
        memcpy (&nodeCount, Buffer+2, 2);
        memcpy (((char *)TreeTableData)+2, Buffer+4, 3);
        memcpy (((char *)TreeTableData)+5, Buffer+8, 4);
        switch (TreeTableData->StreamVersion)
        {
            case 0x8002:
            break;
            case 0x8001:
                if (TreeTableData->NumParams > 4) { TreeTableData->NumParams = 4; }
                if (TreeTableData->NumLocals > 4) { TreeTableData->NumLocals = 4; }
            break;
            case 0x8000:
                TreeTableData->NumParams = 4;
                TreeTableData->NumLocals = 0;
            break;
            default:
            break;
        }
    }
    
    printf("Node Count: %d", nodeCount);
    TreeTableData->NodesBegin = (IFF_TREETABLENODE *)malloc(12 * nodeCount);
    TreeTableData->NodesEnd = TreeTableData->NodesBegin + nodeCount;
    memcpy(TreeTableData->NodesBegin, Buffer+((TreeTableData->StreamVersion == 0x8003) ? 13 : 12), 12 * nodeCount);
    
    return 1;
}