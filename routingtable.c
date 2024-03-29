#include "ne.h"
#include "router.h"


/* ----- GLOBAL VARIABLES ----- */
struct route_entry routingTable[MAX_ROUTERS];
int NumRoutes;


////////////////////////////////////////////////////////////////
void InitRoutingTbl (struct pkt_INIT_RESPONSE *InitResponse, int myID)
{
	int i=0;
	
	NumRoutes=(InitResponse -> no_nbr) + 1;
  	
  	//self link
  	routingTable[0].dest_id = myID;
  	routingTable[0].next_hop = myID;
  	routingTable[0].cost = 0;
  	routingTable[0].path_len = 1; //to self, one router in path
	routingTable[0].path[0] = myID;
  	
  	for(i = 1; i < NumRoutes; i++)
  	{ 
    	int j=i-1;
    	routingTable[i].dest_id = InitResponse -> nbrcost[j].nbr;
    	routingTable[i].next_hop = InitResponse -> nbrcost[j].nbr;
    	routingTable[i].cost = InitResponse -> nbrcost[j].cost;
    	//need to add path variables
    	routingTable[i].path_len = 2; //router to neighbor
		routingTable[i].path[0] = myID;
		routingTable[i].path[1] = InitResponse->nbrcost[j].nbr ;
  	}
  	
  	return;
}

////////////////////////////////////////////////////////////////
int UpdateRoutes(struct pkt_RT_UPDATE *RecvdUpdatePacket, int costToNbr, int myID)
{
	int i; //RecvdUpdatePacket->route iterator
	int j; //routingTable iterator 
	int k; //routingTable->path iterator 
	int origRoutes = NumRoutes; //so tabel dosen't iterate through new entries
	int tabelEdited = 0; //tracks if changes are made to routingTable
	for(i = 0; i < RecvdUpdatePacket->no_routes; i++) //iterate through updated router table
	{
		struct route_entry entryUpdate = RecvdUpdatePacket->route[i];

		int new = 1; //tracks if entry is new to routingTable 
		for(j = 0; j < origRoutes; j++) //iterate through our current router tabel 
		{
			if(routingTable[j].dest_id == entryUpdate.dest_id)
			{
				new = 0;
				if(entryUpdate.cost < routingTable[j].cost) //Path Vecor Rule
				{ 	int ignore = 0;
					for(k = 0; k < entryUpdate.path_len; k++) 
					{
						if(myID == entryUpdate.path[k]) //router found in path, ignore update
						{
							ignore = 1;
							k = entryUpdate.path_len;
						}
					}
					if(ignore != 1) //udate to better route
					{
						routingTable[j].next_hop = RecvdUpdatePacket->sender_id;
						routingTable[j].cost = entryUpdate.cost + costToNbr;
						routingTable[j].path_len = entryUpdate.path_len + 1;
						routingTable[j].path[0] = myID;
						for(k = 0; k < entryUpdate.path_len; k++)
						{
							routingTable[j].path[k+1] = entryUpdate.path[k];
						}
						tabelEdited = 1;
					}
				}
				
				if(routingTable[j].next_hop == RecvdUpdatePacket->sender_id) //forced update rule
				{
					if(entryUpdate.cost > routingTable[j].cost - costToNbr)
					{
						routingTable[j].cost = entryUpdate.cost + costToNbr; //updates to higher cost
						tabelEdited = 1;
					}
				}
			}
		}

		if(new == 1) //New router was found, adding it to routingTable
		{
			routingTable[NumRoutes].dest_id = entryUpdate.dest_id;
			routingTable[NumRoutes].next_hop = RecvdUpdatePacket->sender_id;
			routingTable[NumRoutes].cost = entryUpdate.cost + costToNbr;
			routingTable[NumRoutes].path_len = entryUpdate.path_len + 1;
			routingTable[NumRoutes].path[0] = myID;
			for(k = 0; k < entryUpdate.path_len; k++)
			{
				routingTable[NumRoutes].path[k+1] = entryUpdate.path[k];
			}
			NumRoutes++;
			tabelEdited = 1;
		}
	}
	return tabelEdited;
}


////////////////////////////////////////////////////////////////
void ConvertTabletoPkt(struct pkt_RT_UPDATE *UpdatePacketToSend, int myID)
{
	int i;
	for(i = 0; i < NumRoutes; i++)
	{
		UpdatePacketToSend->route[i] = routingTable[i];
	}
	UpdatePacketToSend->sender_id = myID;
	UpdatePacketToSend->no_routes = NumRoutes;

	//Need to convert from host to network here or main?

	return;
}


////////////////////////////////////////////////////////////////
//It is highly recommended that you do not change this function!
void PrintRoutes (FILE* Logfile, int myID){
	/* ----- PRINT ALL ROUTES TO LOG FILE ----- */
	int i;
	int j;
	for(i = 0; i < NumRoutes; i++){
		fprintf(Logfile, "<R%d -> R%d> Path: R%d", myID, routingTable[i].dest_id, myID);

		/* ----- PRINT PATH VECTOR ----- */
		for(j = 1; j < routingTable[i].path_len; j++){
			fprintf(Logfile, " -> R%d", routingTable[i].path[j]);	
		}
		fprintf(Logfile, ", Cost: %d\n", routingTable[i].cost);
	}
	fprintf(Logfile, "\n");
	fflush(Logfile);
	return;
}


////////////////////////////////////////////////////////////////
void UninstallRoutesOnNbrDeath(int DeadNbr)
{
	int i;
	for(i = 0; i < NumRoutes; i++)
	{
		if(routingTable[i].next_hop == DeadNbr)
		{
			routingTable[i].cost = INFINITY;
		}
	}

	return;
}
