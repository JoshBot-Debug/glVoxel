#pragma once

class Light
{
private:
  unsigned int id;

public:
  Light(unsigned int id);
  virtual ~Light() {}
  
  const unsigned int getID() const;
};