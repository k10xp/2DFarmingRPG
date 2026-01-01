#ifndef NETWORKID_H
#define NETWORKID_H

/// @brief Call this when you deserialize a new net id sent from the server - games won't ever have to call it
/// @param nextVal 
void NetID_DeserializedNewID(int nextVal);

/// @brief Get a new net ID - the authoritative ID an entity has, assigned by the server but guessed by the client
/// @return the network ID, assign to Entity2D::networkID
int NetID_GetID();

#endif