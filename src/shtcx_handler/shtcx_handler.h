#ifndef SHTCX_HANDLER_H_
#define SHTCX_HANDLER_H_

void shtcx_fetch(void);
int32_t shtcx_get_temp(void);
uint32_t shtcx_get_humidity(void);
bool init_shtcx(void);

#endif /* SHTCX_HANDLER_H_ */
