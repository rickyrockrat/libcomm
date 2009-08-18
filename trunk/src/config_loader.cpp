#include "config_loader.h"

#include <fstream>
#include <stdlib.h>

#include "net_address.h"
#include "logger.h"

ConfigLoader* ConfigLoader::self = (ConfigLoader*) NULL;

ConfigLoader *ConfigLoader::getConfigLoader() {
  if (self == (ConfigLoader*) NULL) self = new ConfigLoader();
  return self;
}

ConfigLoader::ConfigLoader() : 
  pathToConfigFile("config/default.conf"), defaultInterface("eth0"),
  interfaceNumber(""), id(-1), participantsIter(0), clientsIter(0) {

  itSelf = (Participant*) NULL;
  //interfacesIpsMapping = NetAddress::getInterfaces();
}

ConfigLoader::~ConfigLoader() {
  std::map<std::string,std::string*>::iterator iter = params.begin();
  for (; iter != params.end(); ++iter) {
    delete iter->second;
  }
  std::vector<Participant*>::iterator iter2 = pi.begin();
  for (; iter2 != pi.end(); ++iter2) {
    delete *iter2;
  }
  std::map<uint16_t,Participant*>::iterator iter3 = clients.begin();
  for (; iter3 != clients.end(); ++iter3) {
    delete iter3->second;
  }
  //delete interfacesIpsMapping;
  std::map<uint16_t, std::map<std::string, std::string*> >::iterator iter4;
  
  for (iter4 = per_process_params.begin(); iter4 != per_process_params.end(); ++iter4) {
    std::map<std::string, std::string*> &ppmap = iter4->second;
    for (iter = ppmap.begin(); iter != ppmap.end(); ++iter) {
      delete iter->second;
    }
  }
}

void ConfigLoader::setPathConfigFile(std::string path) {
  pathToConfigFile = path;
}

void ConfigLoader::setInterfaceNumber(std::string n) {
  interfaceNumber = n;
}

const std::string *ConfigLoader::getParameter(const std::string paramName, int32_t id) const{
  std::map<std::string,std::string*>::const_iterator iter;
  if (id != -1) {
    std::map<uint16_t, std::map<std::string,std::string*> >::const_iterator find = per_process_params.find(id);
    if (find != per_process_params.end()) {
      iter = find->second.find(paramName);     
      if (iter != find->second.end()) {
        return iter->second;
      }
    }
  }
  iter = params.find(paramName);
  if (iter != params.end()) {
    return iter->second;  
  } else {
    return (std::string*) NULL;
  }
}

bool ConfigLoader::getLongParameter(const std::string paramName, 
  uint64_t *value, int32_t id) const
{
  const std::string *valueStr = getParameter(paramName, id); 
  if (valueStr != NULL) {
    std::stringstream ss(*valueStr);
    ss >> *value;
    return true;
  } else {
    return false;
  }
}

bool ConfigLoader::getDoubleParameter(const std::string paramName,
  double *value, int32_t id) const
{
  const std::string *valueStr = getParameter(paramName, id); 
  if (valueStr != NULL) {
    std::stringstream ss(*valueStr);
    ss >> *value;
    return true;
  } else {
    return false;
  }
}

bool ConfigLoader::getBoolParameter(const std::string paramName,
  bool *value, int32_t id) const
{
  const std::string *valueStr = getParameter(paramName, id); 
  if (valueStr != NULL) {
    *value = (*valueStr == "true");
    return true;
  } else {
    return false;
  }
}

const std::vector<Participant*> *ConfigLoader::getPi() const {
  return &pi;
}

size_t ConfigLoader::getSizePi() const {
  return pi.size();
}

const Participant *ConfigLoader::getItSelf() const {
  return itSelf;
}

const Participant *ConfigLoader::getParticipant(uint16_t id) const {
  std::map<uint16_t,Participant*>::const_iterator iter = piId.find(id);
  if (iter == piId.end()) {
    return (Participant*) NULL;
  } else {
    return iter->second;
  }
}

const Participant *ConfigLoader::getParticipant(std::string add) const {
  std::multimap<std::string,Participant*>::const_iterator iter = piAddress.find(add);
  if (iter == piAddress.end()) {
    return (Participant*) NULL;
  } else {
    return iter->second;
  }
}

const Participant *ConfigLoader::getParticipantWithParam(const std::string add,
  const std::string paramName, const std::string paramValue) const {
  std::multimap<std::string,Participant*>::const_iterator iter = piAddress.find(add);
  if (iter == piAddress.end()) {
    return (Participant*) NULL;
  } else {
    Participant *p = NULL;
    const std::string *param;
    
    for (; iter != piAddress.end(); ++iter) {
      p = iter->second;
      param = getParameter(paramName, p->getId());
      if ((param != NULL) && (*param == paramValue)) {
        break;
      }
    }
    return p;
  }
}

const Participant *ConfigLoader::getClient(uint16_t id) const {
  std::map<uint16_t,Participant*>::const_iterator iter = clients.find(id);
  if (iter == piId.end()) {
    return (Participant*) NULL;
  } else {
    return iter->second;
  }
}

size_t ConfigLoader::getSizeClients() const {
  return clients.size();
}

void ConfigLoader::setSelfId(uint16_t id) {
  this->id = id;
}

void ConfigLoader::errorAndQuit(const std::string paramName) {
  Logger::log(ERROR) << "Parameter " << paramName << " not found. Exiting..." 
    << Logger::endm(this);
  exit(-1);
}

void ConfigLoader::loadParameters() {
  std::ifstream file(pathToConfigFile.c_str()); 

  if (file) {
    std::string line;
    while (std::getline(file,line)) {
      proceedLine(line); 
    }
    file.close();
  } else {
    Logger::log(ERROR) << "Error when trying to open the config file "
      << "at location " << pathToConfigFile << Logger::endm(this);
  }

}

bool ConfigLoader::getParamAndValue(std::string &str, std::string *param, std::string *value) {
  std::string paramName;
  size_t s;
  size_t i = 0;
  size_t length;
  
  length = str.length();

  while ((i<length) && 
        ((str[i] == ' ') || (str[i] == '\t') || (str[i] == '\n')) ){
    ++i;
  }
  str = str.substr(i,length);

  s = str.find_first_of(':');
  if (s != std::string::npos) {
      *param = str.substr(0,s);
      str = str.substr(s+1,str.length());

      i = paramName.length();
      
      while ((i>0) && 
        ((paramName[i-1] == ' ') || (paramName[i-1] == '\t'))){
        --i;
      }

      if (i != paramName.length()) {
        paramName.substr(0,i);
      }

      i = 0;
      length = str.length();

      while ((i<length) && 
        ((str[i] == ' ') || (str[i] == '\t'))){
        ++i;
      }
      
      if (i < length) {
        *value = str.substr(i,length);
        return true;
      } else {
        return false;
      }
  } else {
    return false;
  }
}

void ConfigLoader::proceedLine(std::string &line) {
  
  size_t i = 0;
  size_t length = line.length();

  while ((i<length) && 
        ((line[i] == ' ') || (line[i] == '\t') || (line[i] == '\n')) ){
    ++i;
  }

  if (line[i] != '#') {
    std::string interstingPart = line.substr(i,length);
    std::string paramName;
    std::string paramValue;
    if (getParamAndValue(interstingPart, &paramName, &paramValue)) {
       std::string additionalParams;
      
      if (paramName == "default-interface") {
        defaultInterface = paramValue;
      } else if (paramName == "process") {
        size_t pos = paramValue.find_first_of(':');
        /*if (pos != std::string::npos) {
          paramValue = paramValue.substr(0,pos);
        }*/
        

        pos = paramValue.find_first_of(" {");
        if (pos != std::string::npos) {
          additionalParams = paramValue.substr(pos+2, paramValue.length()-pos-3);
          paramValue = paramValue.substr(0, pos);
          proceedAdditionalParams(additionalParams);
        }

        Participant *part = new Participant(participantsIter,paramValue);
        pi.push_back(part);
        piId[participantsIter] = part;
        piAddress.insert(std::pair<std::string,Participant*>(paramValue,part));
        if ((itSelf == (Participant*) NULL) &&
           ((interfaceNumber == "") ? findItSelf(paramValue) 
             : findItSelf(paramValue,interfaceNumber))) {
          itSelf = part;
        }
        ++participantsIter;
      } else if (paramName == "client") {
        size_t pos = paramValue.find_first_of(':');
        if (pos != std::string::npos) {
          paramValue = paramValue.substr(0,pos);
        }
        Participant *part = new Participant(clientsIter,paramValue);
        clients[clientsIter] = part;
        ++clientsIter;
        if ((itSelf == (Participant*) NULL) &&
           ((interfaceNumber == "") ? findItSelf(paramValue) 
             : findItSelf(paramValue,interfaceNumber))) {
          itSelf = part;
        }
      } else if (paramName == "id") {
        params[paramName] = new std::string(paramValue);
        getLongParameter(paramName, (uint64_t*)&id);
      } else {
        params[paramName] = new std::string(paramValue);
      }
    }
  }
}

void ConfigLoader::proceedAdditionalParams(std::string &params) {
  size_t pos;
  
  per_process_params[participantsIter] = std::map<std::string, std::string*>();
  pos = params.find_first_of(';');
  while (pos != std::string::npos) {
    std::string param = params.substr(0,pos);
    params = params.substr(pos+1); 
    std::string paramName;
    std::string paramValue;

    if (getParamAndValue(param, &paramName, &paramValue)) {
      per_process_params[participantsIter][paramName] = new std::string(paramValue);
    }

    pos = params.find_first_of(';');
  }
}

bool ConfigLoader::findItSelf(std::string &paramValue) {
  std::string in("");
  return findItSelf(paramValue,in);
}


bool ConfigLoader::findItSelf(std::string &paramValue,
  std::string &interfaceNumber) {

  if (NetAddress::getLocalHostname() == paramValue) {
    return true;
  } else if ((uint16_t) id == participantsIter) {
    return true;
  } else {
    std::map<std::string,NetAddress>::iterator iter =
      interfacesIpsMapping->find(defaultInterface+":"+interfaceNumber);
    if (iter != interfacesIpsMapping->end()) {
      if (paramValue == iter->second.getAddress()) {
        return true;
      }
    }
  }
  return false;
}
