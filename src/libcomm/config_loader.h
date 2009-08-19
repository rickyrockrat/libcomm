#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H

#include <map>
#include <vector>
#include <string>

#include "participant.h"
#include "net_address.h"

class ConfigLoader {

  public :
    static ConfigLoader *getConfigLoader();
    void setPathConfigFile(std::string path);
    void setInterfaceNumber(std::string n);
    const std::string *getParameter(const std::string paramName, int32_t id=-1) const; 
    bool getLongParameter(const std::string paramName, uint64_t *value, int32_t id=-1) const;
    bool getDoubleParameter(const std::string paramName, double *value, int32_t id=-1) const;
    bool getBoolParameter(const std::string paramName, bool *value, int32_t id=-1) const;
    const std::vector<Participant*> *getPi() const;
    size_t getSizePi() const;
    const Participant *getItSelf() const;
    const Participant *getParticipant(uint16_t id) const;
    const Participant *getParticipant(std::string address) const;
    const Participant *getParticipantWithParam(const std::string add, 
      const std::string paramName, const std::string paramValue) const;
    const Participant *getClient(uint16_t id) const;          
    size_t getSizeClients() const;
    void setSelfId(uint16_t id);
    void errorAndQuit(const std::string paramName);


  private :
    static ConfigLoader *self;
    std::string pathToConfigFile;
    std::string defaultInterface;
    std::string interfaceNumber;
    std::map<std::string,std::string*> params;
    std::map<uint16_t, std::map<std::string, std::string*> > per_process_params;
    std::map<std::string,NetAddress> *interfacesIpsMapping;
    
    Participant *itSelf;
    int32_t id;
    
    size_t participantsIter;
    std::vector<Participant*> pi; 
    std::map<uint16_t, Participant*> piId;
    std::multimap<std::string, Participant*> piAddress;

    size_t clientsIter;
    std::map<uint16_t,Participant*> clients;

    ConfigLoader();
    ~ConfigLoader();
    void loadParameters();
    bool getParamAndValue(std::string &str, std::string *param, std::string *value);
    void proceedLine(std::string &line);
    void proceedAdditionalParams(std::string &params);
    bool findItSelf(std::string &paramValue);
    bool findItSelf(std::string &paramValue, std::string &interfaceNumber);

    friend class libcomm; 
};

#endif
