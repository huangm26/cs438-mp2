#include "Manager.h"


Manager::Manager()
{
	readTopologFile();
	readMessageFile();


}


Manager::~Manager()
{
}

Manager::readTopologFile()
{
	FILE * topology_file;

	topology_file = fopen ("topology.txt","r");
	if(topology_file == NULL){
		printf("Error opening topology file");
		return -1;
	}
}

Manager::readMessageFile()
{
	FILE * message_file;

	message_file = fopen ("message.txt","r");
	if(message_file == NULL){
		printf("Error opening message file");
		return -1;
	}

}