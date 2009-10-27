int writeToFile(Player* tony){
    FILE *fd = fopen("testfile.txt","w+");
    if(fd){
	fprintf(fd,"%d %d %d %d",tony->hp,tony->exp,tony->x,tony->y);
	fclose(fd);
	return 1;
    }
    return 0;

}

