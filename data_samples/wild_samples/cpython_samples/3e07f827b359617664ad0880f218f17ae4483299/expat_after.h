typedef struct {
  enum XML_FeatureEnum feature;
  const XML_LChar *name;
  long int value;
} XML_Feature;

XMLPARSEAPI(const XML_Feature *)
XML_GetFeatureList(void);

#ifdef XML_DTD
/* Added in Expat 2.4.0. */
XMLPARSEAPI(XML_Bool)
XML_SetBillionLaughsAttackProtectionMaximumAmplification(
    XML_Parser parser, float maximumAmplificationFactor);

/* Added in Expat 2.4.0. */
XMLPARSEAPI(XML_Bool)
XML_SetBillionLaughsAttackProtectionActivationThreshold(
    XML_Parser parser, unsigned long long activationThresholdBytes);
#endif

/* Expat follows the semantic versioning convention.
   See http://semver.org.
*/
#define XML_MAJOR_VERSION 2
#define XML_MINOR_VERSION 5
#define XML_MICRO_VERSION 0

#ifdef __cplusplus
}
#endif

#endif /* not Expat_INCLUDED */