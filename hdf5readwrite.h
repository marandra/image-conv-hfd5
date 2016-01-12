std::vector<std::string> &split(const std::string &, char , std::vector<std::string> &);
std::vector<std::string> split(const std::string &, char );
int writefile(std::string vTmpFile, const std::vector<char>& vDataSet);
int extractfromh5(struct param, std::string vTmpFile);
int writetoh5(std::string, struct param);
std::string hdf5read(std::string vFileName);
