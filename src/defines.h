#ifndef PAW_PRINT_DEFINES
#define PAW_PRINT_DEFINES


#define null 0

#define PAW_GETTER(TYPE, VAR_NAME) \
  inline const TYPE VAR_NAME() const { return VAR_NAME##_; }


#endif
