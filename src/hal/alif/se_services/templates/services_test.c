/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
/**
 * @file  services_test.c
 * @brief Services library test harness
 * @ingroup services
 * @par
 */

/******************************************************************************
 *  I N C L U D E   F I L E S
 *****************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "services_lib_api.h"
#ifdef A32_LINUX
#include "services_lib_linux.h"
#else
#include "services_lib_interface.h"
#endif

void test_crypto(uint32_t services_handle);

/* forward tests */
static uint32_t test_services_heartbeat(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_pinmux(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_padcontrol(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_crypto_trng64(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_crypto_trng32(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_crypto_lcs(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_get_se_revision(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_get_socid(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_read_otp (char *p_test_name, uint32_t services_handle);
static uint32_t test_services_read_otp_illegal (char *p_test_name, uint32_t services_handle);
static uint32_t test_services_get_device_data(char *p_test_name,uint32_t services_handle);
static uint32_t test_services_gettoc(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_gettoc_via_name_m55_he(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_gettoc_via_name_m55_hp(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_gettoc_via_cpuid_he(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_gettoc_via_cpuid_hp(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_gettoc_via_cpuid_a32(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_gettoc_version(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_gettoc_data(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_boot_toc_a32(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_boot_cpu(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_boot_reset_cpu(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_boot_release_extsys0(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_mbedtls_aes(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_mbedtls_sha(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_bounds(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_mem_retention_config(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_ewic_config(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_vbat_wakeup_config(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_ospi_write_key(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_boot_reset_soc(char *p_test_name, uint32_t services_handle);

static uint32_t test_services_select_osc_source(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_select_pll_source(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_enable_clock(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_set_ES0_frequency(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_set_ES1_frequency(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_select_a32_source(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_select_aclk_source(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_set_divider(char *p_test_name, uint32_t services_handle);

static uint32_t test_services_pll_deinit(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_pll_initialize(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_xtal_start(char *p_test_name, uint32_t services_handle);/*unused*/
static uint32_t test_services_xtal_stop(char *p_test_name, uint32_t services_handle); /*unused*/
static uint32_t test_services_xtal_is_started(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_clkpll_start(char *p_test_name, uint32_t services_handle);/*unused*/
static uint32_t test_services_clkpll_stop(char *p_test_name, uint32_t services_handle); /*unused*/
static uint32_t test_services_clkpll_is_locked(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_pll_xtal(char *p_test_name, uint32_t services_handle);

static uint32_t test_services_cpu_boot_sequence(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_dcdc_voltage(char *p_test_name,
                                           uint32_t services_handle);
static uint32_t test_services_ldo_voltage(char *p_test_name,
                                          uint32_t services_handle);

static uint32_t test_services_get_bus_frequencies(char *p_test_name, uint32_t services_handle);
static uint32_t test_services_get_eui(char *p_test_name, uint32_t services_handle);

static uint32_t test_services_bor_en(char *p_test_name, uint32_t services_handle);

/*******************************************************************************
 *  M A C R O   D E F I N E S
 ******************************************************************************/
#define RANDOMIZER_FEATURE          0
#define NUMBER_OF_TEST_RUNS         1   /* Number of times to test            */

#define SANITY_TESTS_ENABLE         1   /* Enable the sanity tests run as part of release builds */
#define A32_BOOT_WORKAROUND         1   /* Skip A32 boot tests that crash the current B0 device */
#define PLL_XTAL_TESTS_ENABLE       0
#define CPU_BOOT_SEQUENCE_TEST_ENABLE 0 /* Boot a CPU core using the low level APIs */

#if defined(M55_HE)
#define CPU_STRING "M55_HE"
#elif defined(M55_HP)
#define CPU_STRING "M55_HP"
#elif defined(A32)
#define CPU_STRING "A32"
#else
#define CPU_STRING "<unknown>"
#endif

/**
 * @brief TOC flags bit values
 */
#define TOC_IMAGE_COMPRESSED          0x10u
#define TOC_IMAGE_LOAD                0x20u
#define TOC_IMAGE_BOOT                0x40u
#define TOC_IMAGE_ENCRYPT             0x80u
#define TOC_IMAGE_DEFERRED            0x100u

/**
 * @brief Flag positions with the flag string
 */
#define FLAG_STRING_COMPRESSED 0
#define FLAG_STRING_LOAD_IMAGE 1
#define FLAG_STRING_VERIFY     2
#define FLAG_STRING_CPU_BOOTED 3
#define FLAG_STRING_ENCRYPTED  4
#define FLAG_STRING_DEFERRED   5
#define FLAG_STRING_END        6
#define FLAG_STRING_SIZE       10


#define PRINT_TEST_RESULT   TEST_print(services_handle, \
                                       "** TEST %s error_code=%s service_resp=0x%08X\n", \
                                       p_test_name,                  \
                                       SERVICES_error_to_string(error_code), \
                                       service_error_code)


/*******************************************************************************
 *  T Y P E D E F S
 ******************************************************************************/

/**
 * @struct services_test_t
 * @brief  test entry description
 */
typedef struct
{
  uint32_t (*test_fn)(char *p_test_name, uint32_t services_handle);
  char *test_name;
  bool enabled;
} services_test_t;

/*******************************************************************************
 *  G L O B A L   V A R I A B L E S
 ******************************************************************************/

static services_test_t s_tests[] =
{
    { test_services_heartbeat,               "heartbeat       "            , false}, /*0*/
    { test_services_pinmux,                  "pinmux          "            , false}, /*1*/
    { test_services_padcontrol,              "padcontrol      "            , false}, /*2*/

    { test_services_crypto_trng64,           "crypto TRNG 64  "            , false}, /*3*/
    { test_services_crypto_trng32,           "crypto TRNG 32  "            , false}, /*4*/
    { test_services_crypto_trng64,           "crypto TRNG 64  "            , false}, /*5*/
    { test_services_mbedtls_sha,             "MbedTLS SHA     "            , false}, /*6*/
    { test_services_crypto_lcs,              "crypto get LCS  "            , false}, /*7*/

    { test_services_get_se_revision,         "get SE revision "            , false}, /*8*/
    { test_services_get_socid,               "get soc id      "            , false}, /*9*/
    { test_services_get_device_data,         "get device info"             , false}, /*10*/

    { test_services_read_otp,                "OTP read otp        "        , false}, /*11*/
    { test_services_read_otp_illegal,        "OTP read otp illegal"        , false}, /*12*/

    { test_services_gettoc,                  "TOC get data    "            , false}, /*13*/
    { test_services_gettoc_via_name_m55_he,  "TOC via name  HE"            , false}, /*14*/
    { test_services_gettoc_via_name_m55_hp,  "TOC via name  HP"            , false}, /*15*/
    { test_services_gettoc_via_cpuid_he,     "TOC via cpuid HE"            , false}, /*16*/
    { test_services_gettoc_via_cpuid_hp,     "TOC via cpuid HP"            , false}, /*17*/
    { test_services_gettoc_via_cpuid_a32,    "TOC via cpuidA32"            , false}, /*18*/
    { test_services_gettoc_version,          "TOC version     "            , false}, /*19*/
    { test_services_gettoc_data,             "TOC get data    "            , false}, /*20*/

    { test_services_boot_toc_a32,            "Boot TOC A32    "            , false}, /*21*/
    { test_services_boot_cpu,                "Boot CPU        "            , false}, /*22*/
    { test_services_boot_reset_cpu,          "Boot reset CPU  "            , false}, /*23*/
    { test_services_boot_release_extsys0,    "Release EXTSYS0 "            , false}, /*24*/
    { test_services_mbedtls_aes,             "MbedTLS AES     "            , false}, /*25*/
    { test_services_bounds,                  "Bounds Tests    "            , false}, /*26*/
    { test_services_mem_retention_config,    "SE Memory Retention config"  , false}, /*27*/
    { test_services_ewic_config,             "EWIC config"                 , false}, /*28*/
    { test_services_vbat_wakeup_config,      "VBAT Wake Up config"         , false}, /*29*/
    { test_services_pll_deinit,              "PLL De-init             "    , false}, /*30*/
    { test_services_pll_initialize,          "PLL Initialize          "    , false}, /*31*/
    { test_services_xtal_is_started,         "XTAL check              "    , false}, /*32*/
    { test_services_clkpll_is_locked,        "PLL Check               "    , false}, /*33*/
    { test_services_ospi_write_key,          "OSPI write key          "    , false}, /*34*/
    { test_services_set_ES0_frequency,       "Set ES0 clock frequency "    , false}, /*35*/
    { test_services_set_ES1_frequency,       "Set ES1 clock frequency "    , false}, /*36*/
    { test_services_select_osc_source,       "Select oscillator source"    , false}, /*37*/
    { test_services_select_pll_source,       "Select PLL source       "    , false}, /*38*/
    { test_services_enable_clock,            "Enable/Disable clock    "    , false}, /*39*/
    { test_services_select_a32_source,       "Select A32 source       "    , false}, /*40*/
    { test_services_select_aclk_source,      "Select ACLK source      "    , false}, /*41*/
    { test_services_set_divider,             "Set a clock divider     "    , false}, /*42*/
    { test_services_boot_reset_soc,          "Reset SOC               "    , false}, /*43*/

    { test_services_pll_xtal,                "Test PLL/XTAL sequences"     , false}, /*44*/
    { test_services_clkpll_start,            "Test CLK/PLL start     "     , false}, /*45*/
    { test_services_clkpll_stop,             "Test CLK/PLL stop      "     , false}, /*46*/
    { test_services_xtal_start,              "Test XTAL start        "     , false}, /*47*/
    { test_services_xtal_stop,               "Test XTAL stop         "     , false}, /*48*/

    { test_services_cpu_boot_sequence,       "Test CPU boot sequence "     , false},  /*49*/
    { test_services_dcdc_voltage,            "DCDC voltage control   "     , false},  /*50*/
    { test_services_ldo_voltage,             "LDO voltage control    "     , false},  /*51*/
    { test_services_get_bus_frequencies,     "Get BUS frequencies    "     , false},  /*52*/
    { test_services_get_eui,                 "Get EUI-48/EUI-64 extensions", false},  /*53*/
    { test_services_bor_en,                  "BOR_EN control",               false},  /*54*/
};

static SERVICES_toc_data_t     toc_info;    /*!< Global to test harness */
static SERVICES_version_data_t device_data; /*!< Global to test harness */

/*******************************************************************************
 *  C O D E
 ******************************************************************************/

/**
 * @brief CPU ID to return string
 * @param cpu_id
 * @return
 */
static char *CPUID_to_string(uint32_t cpu_id)
{
  static char err_string[10] = { 0 };
  char *p_str = NULL;

  switch (cpu_id)
   {
       case HOST_CPU_0:
         p_str = "A32_0";
         break;
       case HOST_CPU_1:
         p_str = "A32_1";
         break;
       case EXTSYS_0:
         p_str = "M55_HP";
         break;
       case EXTSYS_1:
         p_str = "M55_HE";
         break;
       case 15:
         p_str = "CM0+  ";
         break;
       default:
         p_str = "??????";
         break;
  }
  strncpy(err_string, p_str, sizeof(err_string));

  return (char *)&err_string[0];
}

/**
 * @brief flags_to_string - convert TOC 'Flags' to a string
 * @param p_status
 * @param flag_string
 * @return
 */
char *flags_to_string(uint32_t flags, char flag_string[])
{
  bool compressed = (flags & TOC_IMAGE_COMPRESSED) != 0;
  flag_string[FLAG_STRING_COMPRESSED] = compressed == true ? 'C' : 'u';

  bool load = (flags & TOC_IMAGE_LOAD) != 0;
  flag_string[FLAG_STRING_LOAD_IMAGE] = load ? 'L' : ' ';

  bool booted = (flags & TOC_IMAGE_BOOT) != 0;
  flag_string[FLAG_STRING_CPU_BOOTED] = booted ? 'B' : ' ';

  bool encrypted = (flags & TOC_IMAGE_ENCRYPT) != 0;
  flag_string[FLAG_STRING_ENCRYPTED] = encrypted ? 'E' : ' ';

  bool deferred = (flags & TOC_IMAGE_DEFERRED) != 0x0;
  flag_string[FLAG_STRING_DEFERRED] = deferred ? 'D' : ' ';

#if 0
  bool verify = (flags & TOC_IMAGE_DEFERRED) != 0x0;
  flag_string[FLAG_STRING_VERIFY] = p_status->verified == true ? 'V' : 's';
#endif

  flag_string[FLAG_STRING_END] = '\0';
  return (char *)&flag_string[0]; /*!< return string back to printing */
}

/**
 * @fn     static uint32_t digit_count(uint32_t digits)
 * @brief  Count up the number of digits
 * @param  digits
 * @return the number of digits
 */
static uint32_t digit_count(uint32_t digits)
{
  uint32_t count = 0;

  do
  {
    digits /= 10;
    count = count + 1;
  } while (digits != 0);

  return count;
}

/**
 * @brief
 *
 * @param major
 * @param minor
 * @param patch
 * @return
 */
static char *version_string_pack(uint32_t major, uint32_t minor, uint32_t patch)
{
  static char version_string[12] = {0}; /* formatted */
  uint32_t major_length;        /* length of characters */
  uint32_t minor_length;        /* length of characters */
  uint32_t patch_length;        /* length of characters */

  major_length = digit_count(major);
  minor_length = digit_count(minor);
  patch_length = digit_count(patch);

  sprintf(version_string, "%*d.%*d.%*d",
          (int)major_length, (int)major,
          (int)minor_length, (int)minor,
          (int)patch_length, (int)patch);

  return (char *)&version_string[0];
}

/**
 * @brief
 *
 * @param dest
 * @param src
 * @param num_bytes
 */
static void format_and_print(char *dest, uint8_t *src, int num_bytes)
{
  int next_pos = 0;

  for (int i=0; i < num_bytes; i++)
  {
    next_pos += sprintf((char *)(dest+next_pos), "%X", src[i]);
  }
}

/**
  * APPLICATION Services
  */

/**
 * @fn    static uint32_t test_services_pinmux(uint32_t services_handle)
 * @param p_test_name
 * @param services_handle
 * @return
 */
static uint32_t test_services_pinmux(char*p_test_name,
                                     uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

#if ((DEVICE_TYPE == FUSION && DEVICE_REVISION == REV_B0) \
    || (DEVICE_TYPE == SPARK))
  error_code = SERVICES_pinmux(services_handle,
                               1, 4, 1,
                               &service_error_code);
#else
   /* configures blinky */
  error_code = SERVICES_pinmux(services_handle,
                               1, 14, 0,
                               &service_error_code);
#endif

  TEST_print(services_handle,
             "** TEST %s error_code=%s service_resp=0x%08X %s\n",
             p_test_name,
             SERVICES_error_to_string(error_code), 
             service_error_code, 
             service_error_code == PINMUX_SUCCESS ? "" : "\tINVALID PIN");

  return error_code;
}

/**
 * @fn    static uint32_t test_services_padcontrol(uint32_t services_handle)
 * @param p_test_name
 * @param services_handle
 * @return
 */
static uint32_t test_services_padcontrol(char*p_test_name,
                                         uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

#if ((DEVICE_TYPE == FUSION && DEVICE_REVISION == REV_B0) \
    || (DEVICE_TYPE == SPARK))
  error_code = SERVICES_padcontrol(services_handle,
                                   1, 4, 0x23,
                                   &service_error_code);
#else
   /* configures blinky */
  error_code = SERVICES_padcontrol(services_handle,
                                   1, 14, 0x0,
                                   &service_error_code);
#endif

  TEST_print(services_handle,
             "** TEST %s error_code=%s service_resp=0x%08X %s\n",
             p_test_name,
             SERVICES_error_to_string(error_code),
             service_error_code, 
             service_error_code == PINMUX_SUCCESS ? "" : "\tINVALID PIN");

  return error_code;
}

/**
 * @fn    static uint32_t test_services_crypto_trng64(uint32_t services_handle)
 * @param services_handle
 * @return
 */
static uint32_t test_services_crypto_trng64(char *p_test_name,
                                            uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  int32_t service_error_code;
  uint64_t rnd_value;

  error_code = SERVICES_cryptocell_get_rnd(services_handle,
                                           sizeof(uint64_t),/* random number/vector length in bytes*/
                                           &rnd_value,
                                           &service_error_code);
  TEST_print(services_handle,
              "** TEST %s error_code=%s 64-bit Random value = 0x%jx service_resp=0x%08X\n",
              p_test_name,
              SERVICES_error_to_string(error_code),
              rnd_value,
              service_error_code);

  return (error_code);
}

/**
 * @fn      static uint32_t test_services_crypto_trng64(uint32_t services_handle)
 * @param services_handle
 * @return
 */
static uint32_t test_services_crypto_trng32(char *p_test_name,
                                            uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  int32_t service_error_code;
  uint32_t rnd_value;

  error_code = SERVICES_cryptocell_get_rnd(services_handle,
                                           sizeof(uint32_t),/* random number/vector length in bytes*/
                                           &rnd_value,
                                           &service_error_code);
  TEST_print(services_handle,
             "** TEST %s error_code=%s 32-bit Random value = 0x%08x service_resp=0x%08X\n",
             p_test_name,
             SERVICES_error_to_string(error_code),
             rnd_value,
             service_error_code);

  return error_code;
}

/**
 * Crypto Services
 */
/**
 * @fn    static uint32_t test_services_crypto_lcs(uint32_t services_handle)
 * @param services_handle
 * @return
 */
static uint32_t test_services_crypto_lcs(char *p_test_name,
                                         uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  int32_t service_error_code;
  uint32_t lcs_state = 0xdeadbeef;

  error_code = SERVICES_cryptocell_get_lcs(services_handle,
                                           &lcs_state,
                                           &service_error_code);
  TEST_print(services_handle,
             "** TEST %s error_code=%s service_resp=0x%08X\n",
             p_test_name,
             SERVICES_error_to_string(error_code),
             lcs_state,
             service_error_code);

  return (error_code);
}

/**
 *  MAINTENANCE Services
 */

/**
 * @brief get otp data using read OTP Service
 *
 * @param p_test_name
 * @param services_handle
 * @return
 * @note  Use knowledge of OTP eFuse map to read the Device Part#
 */
static uint32_t test_services_read_otp(char *p_test_name,
                                       uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;
  uint32_t otp_value;

   /**
    * Customer Security Flags field
    * OTP Word Address = 0x0005F, Size = 4 bytes
    */
   error_code = SERVICES_system_read_otp(services_handle,
                                         OTP_CUSTOMER_SECURITY_FLAGS_START,
                                         &otp_value,
                                         &service_error_code);
   PRINT_TEST_RESULT;

   return error_code;
}

/**
 * @brief get otp data using read OTP Service
 *
 * @param p_test_name
 * @param services_handle
 * @return
 */
static uint32_t test_services_read_otp_illegal(char *p_test_name,
                                               uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;
  uint32_t otp_value;

  /**
   * Read an illegal (no permission) offset you should not be trying
   */
  error_code = SERVICES_system_read_otp(services_handle,
                                          0x00,
                                          &otp_value,
                                          &service_error_code);
  PRINT_TEST_RESULT;

  return (error_code);
}

static uint32_t test_services_heartbeat(char *p_test_name,
                                        uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;

  error_code = SERVICES_heartbeat(services_handle);

  TEST_print(services_handle, "** TEST %s error_code=%s\n",
                              p_test_name,                  \
                              SERVICES_error_to_string(error_code));
  return (error_code);
}

/**
  * System management Services
  */
static uint32_t test_services_gettoc(char *p_test_name,
                                     uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;
  uint32_t number_of_tocs = 0;

  error_code = SERVICES_system_get_toc_number(services_handle,
                                              &number_of_tocs,
                                              &service_error_code);
  TEST_print(services_handle,
             "** TEST %s error_code=%s Application TOC number = %d " \
             "service_resp=0x%08X\n",
             p_test_name,
             SERVICES_error_to_string(error_code),
             number_of_tocs,
             service_error_code);

  return (error_code);
}

/**
 * @brief Read OTP
 *
 * @param p_test_name
 * @param services_handle
 * @return
 */
/**
 * @brief TOC Test - get all TOC data
 *
 * @param p_test_name
 * @param services_handle
 * @return
 */
static uint32_t test_services_gettoc_data(char *p_test_name,
                                          uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;
  uint32_t each_toc;

  error_code = SERVICES_system_get_toc_data(services_handle,
                                            &toc_info,
                                            &service_error_code);
  TEST_print(services_handle,
             "** TEST %s error_code=%s TOC number = %d service_resp=0x%08X\n",
             p_test_name,
             SERVICES_error_to_string(error_code),
             toc_info.number_of_toc_entries,
             service_error_code);

  TEST_print(services_handle,
             "+---------------------------------------------------------------------------------+\n");
  TEST_print(services_handle,
             "|   Name   |    CPU   |Load Address|Boot Address|Image Size| Version |    Flags   |\n");
  TEST_print(services_handle,
             "+---------------------------------------------------------------------------------+\n");

  for (each_toc = 0; each_toc < toc_info.number_of_toc_entries ; each_toc++)
  {
    char flags_string[FLAG_STRING_SIZE] = {0}; /* Flags as string   */

    TEST_print(services_handle,
               "| %8s |  %6s  | 0x%08X | 0x%08X | %8d | %6s  | %10s |\n",
               toc_info.toc_entry[each_toc].image_identifier,
               CPUID_to_string(toc_info.toc_entry[each_toc].cpu),
               toc_info.toc_entry[each_toc].load_address,
               toc_info.toc_entry[each_toc].boot_address,
               toc_info.toc_entry[each_toc].image_size,
               version_string_pack(
                   (toc_info.toc_entry[each_toc].version >> 24) & 0xFF,
                   (toc_info.toc_entry[each_toc].version >> 16) & 0xFF,
                   (toc_info.toc_entry[each_toc].version >>  8) & 0xFF),
               flags_to_string(toc_info.toc_entry[each_toc].flags,
                               flags_string));
  }
  TEST_print(services_handle,
             "+---------------------------------------------------------------------------------+\n");

  return error_code;
}

/**
 * @brief * @brief TOC test - get via name M55_HE
 *
 * @param p_test_name
 * @param services_handle
 * @return
 */
static uint32_t test_services_gettoc_via_name_m55_he(char *p_test_name,
                                                     uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;

#if CPU != A32
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  error_code = SERVICES_system_get_toc_via_name(services_handle,
                                                (uint8_t *)"M55-HE",
                                                &service_error_code);
  TEST_print(services_handle,
             "** TEST %s error_code=%s service_resp=0x%08X\n",
             p_test_name,
             SERVICES_error_to_string(error_code),
             service_error_code);

  return error_code;
#else
  (void)(p_test_name);
  (void)(services_handle);
  error_code = SERVICES_RESP_UNKNOWN_COMMAND;
#endif // #if CPU != A32
  return error_code;
}

/**
 * @brief TOC test - get via name M55_HP
 *
 * @param p_test_name
 * @param services_handle
 * @return
 */
static uint32_t test_services_gettoc_via_name_m55_hp(char *p_test_name,
                                                     uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;

#if CPU != A32
  uint32_t service_error_code;

  error_code = SERVICES_system_get_toc_via_name(services_handle,
                                                (uint8_t *)"M55-HP",
                                                &service_error_code);
  TEST_print(services_handle,
             "** TEST %s error_code=%s service_resp=0x%08X\n",
             p_test_name,
             SERVICES_error_to_string(error_code),
             service_error_code);

#else
  (void)(p_test_name);
  (void)(services_handle);

  error_code = SERVICES_RESP_UNKNOWN_COMMAND;
#endif // #if CPU != A32
  return error_code;
}

/**
 * @brief TOC Test - get version
 *
 * @param p_test_name
 * @param services_handle
 * @return
 */
static uint32_t test_services_gettoc_version(char *p_test_name,
                                             uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t version = 0;                  /*<! Returned version */
  uint32_t service_error_code;

  error_code = SERVICES_system_get_toc_version(services_handle,
                                               &version,
                                               &service_error_code);
  TEST_print(services_handle,
             "** TEST %s error_code=%s service_resp=0x%08X version=%X\n",
             p_test_name,
             SERVICES_error_to_string(error_code),
             service_error_code,
             version);

  return error_code;
}

/**
 * @brief SYSTEM Test - get TOC data via CPU ID HE
 * @param p_test_name
 * @param services_handle
 * @return
 */
static uint32_t test_services_gettoc_via_cpuid_he(char *p_test_name,
                                                  uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  error_code = SERVICES_system_get_toc_via_cpuid(services_handle,
                                                 EXTSYS_1,
                                                 &toc_info,
                                                 &service_error_code);

  TEST_print(services_handle,
             "** TEST %s error_code=%s Found %d TOCs for CPU\n",
             p_test_name,
             SERVICES_error_to_string(error_code),
             toc_info.number_of_toc_entries);
  for (uint32_t each_toc = 0; each_toc < toc_info.number_of_toc_entries; each_toc++)
  {
    SERVICES_toc_info_t *toc_entry_p;

    toc_entry_p = (SERVICES_toc_info_t *)&toc_info.toc_entry[each_toc];

    TEST_print(services_handle,
               "    -> Name %8s flags %X Version %X %X %X\n",
               toc_entry_p->image_identifier,
               toc_entry_p->flags,
               ((toc_entry_p->version >> 24) & 0x0F),
               ((toc_entry_p->version >> 16) & 0x0F),
               ((toc_entry_p->version >>  8) & 0x0F));
  }

  return error_code;
}

/**
 * @brief SYSTEM Test - get TOC data via CPU ID HP
 * @param p_test_name
 * @param services_handle
 * @return
 */
static uint32_t test_services_gettoc_via_cpuid_hp(char *p_test_name,
                                                  uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  error_code = SERVICES_system_get_toc_via_cpuid(services_handle,
                                                 EXTSYS_0,
                                                 &toc_info,
                                                 &service_error_code);
  TEST_print(services_handle,
             "** TEST %s error_code=%s Found %d TOCs for CPU\n",
             p_test_name,
             SERVICES_error_to_string(error_code),
             toc_info.number_of_toc_entries);
  for (uint32_t each_toc = 0; each_toc < toc_info.number_of_toc_entries; each_toc++)
  {
    SERVICES_toc_info_t *toc_entry_p;

    toc_entry_p = (SERVICES_toc_info_t *)&toc_info.toc_entry[each_toc];

    TEST_print(services_handle,
               "    -> Name %8s flags %X Version %X %X %X\n",
               toc_entry_p->image_identifier,
               toc_entry_p->flags,
               ((toc_entry_p->version >> 24) & 0x0F),
               ((toc_entry_p->version >> 16) & 0x0F),
               ((toc_entry_p->version >>  8) & 0x0F));
  }

  return error_code;
}

/**
 * @brief SYSTEM Test - get TOC data via CPU ID A32
 * @param p_test_name
 * @param services_handle
 * @return
 */
static uint32_t test_services_gettoc_via_cpuid_a32(char *p_test_name,
                                                  uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  error_code = SERVICES_system_get_toc_via_cpuid(services_handle,
                                                 HOST_CPU_0,
                                                 &toc_info,
                                                 &service_error_code);
  TEST_print(services_handle,
             "** TEST %s error_code=%s Found %d TOCs for CPU\n",
             p_test_name,
             SERVICES_error_to_string(error_code),
             toc_info.number_of_toc_entries);
  for (uint32_t each_toc = 0; each_toc < toc_info.number_of_toc_entries; each_toc++)
  {
    SERVICES_toc_info_t *toc_entry_p;

    toc_entry_p = (SERVICES_toc_info_t *)&toc_info.toc_entry[each_toc];

    TEST_print(services_handle,
               "    -> Name %8s flags %X Version %X %X %X\n",
               toc_entry_p->image_identifier,
               toc_entry_p->flags,
               ((toc_entry_p->version >> 24) & 0x0F),
               ((toc_entry_p->version >> 16) & 0x0F),
               ((toc_entry_p->version >>  8) & 0x0F));
  }

  return error_code;
}

/**
 * @brief Boot Services - get SE revision
 * @param p_test_name
 * @param services_handle
 * @return
 */
static uint32_t test_services_get_se_revision(char *p_test_name,
                                              uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code = 0;
  uint8_t se_revision[80] = {0};

  error_code = SERVICES_get_se_revision(services_handle,
                                        (uint8_t*)&se_revision[0],
                                        &service_error_code);
  TEST_print(services_handle,
             "** TEST %s error_code=%s service_resp=0x%08X\n",
             p_test_name,
             SERVICES_error_to_string(error_code),
             service_error_code);
  TEST_print(services_handle,
             "     ** %s\n",
             se_revision);

  return error_code;
}

static uint32_t test_services_get_socid(char *p_test_name,
                                        uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;
  uint32_t device_part_number;

  error_code = SERVICES_system_get_device_part_number(services_handle,
                                                      &device_part_number,
                                                      &service_error_code);
  TEST_print(services_handle,
             "** TEST %s error_code=%s Device number 0x%X service_resp=0x%08X\n",
             p_test_name,
             SERVICES_error_to_string(error_code),
             device_part_number,
             service_error_code);

  return error_code;
}

static uint32_t test_services_get_device_data(char *p_test_name,
                                              uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;
  static uint8_t print_buffer[65] = {0}; /*Prefix + 32 character data*/

  error_code = SERVICES_system_get_device_data(services_handle,
                                               &device_data,
                                               &service_error_code);
  TEST_print(services_handle,
             "** TEST %s  error_code=%s service_resp=0x%08X\n",
             p_test_name,
             SERVICES_error_to_string(error_code),
             service_error_code);

  TEST_print(services_handle,"     ** Revision  %X\n", device_data.revision_id);
  memcpy((uint8_t*)&print_buffer[0], device_data.ALIF_PN,
         sizeof(device_data.ALIF_PN));
  TEST_print(services_handle,"     ** ALIF_PN   %s\n", print_buffer);

  format_and_print((char*)&print_buffer[0],
		  	  	   &device_data.HBK0[0],sizeof(device_data.HBK0));
  TEST_print(services_handle,"     ** HBK0      %s\n", print_buffer);

  format_and_print((char*)&print_buffer[0],
                    &device_data.HBK1[0],sizeof(device_data.HBK1));
  TEST_print(services_handle,"     ** HBK1      %s\n", print_buffer);

  format_and_print((char*)&print_buffer[0],
                    &device_data.HBK_FW[0],sizeof(device_data.HBK_FW));
  TEST_print(services_handle,"     ** HBK_FW    %s\n", print_buffer);

  format_and_print((char*)&print_buffer[0],
                    &device_data.config[0],sizeof(device_data.config));
  TEST_print(services_handle,"     ** Wounding  %s\n", print_buffer);

  format_and_print((char*)&print_buffer[0],
                    &device_data.MfgData[0],sizeof(device_data.MfgData));
  TEST_print(services_handle,"     ** MfgData   %s\n", print_buffer);

  format_and_print((char*)&print_buffer[0],
                    &device_data.SerialN[0],sizeof(device_data.SerialN));
  TEST_print(services_handle,"     ** Serial#   %s\n", print_buffer);

  TEST_print(services_handle,"     ** LCS       %X\n", device_data.LCS);

  return error_code;
}

static uint32_t test_services_ewic_config(char *p_test_name,
                                          uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;

  error_code = SERVICES_power_ewic_config(services_handle,
                                          EWIC_RTC_SE | EWIC_RTC_A,
                                          OFF_PROFILE);

  TEST_print(services_handle,
             "** TEST %s error_code=%s\n",
             p_test_name,
             SERVICES_error_to_string(error_code));

  return error_code;

}

static uint32_t test_services_vbat_wakeup_config(char *p_test_name,
                                                 uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;

  error_code = SERVICES_power_wakeup_config(services_handle,
                                            WE_SERTC
                                            | WE_LPRTC,
                                            OFF_PROFILE);

  TEST_print(services_handle,
             "** TEST %s error_code=%s\n",
             p_test_name,
             SERVICES_error_to_string(error_code));

  return error_code;
}

static uint32_t test_services_mem_retention_config(char *p_test_name,
                                                      uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
#if 0
  error_code = SERVICES_power_mem_retention_config(services_handle,
                                                   SERAM_MASK,
                                                   OFF_PROFILE);
#endif
  TEST_print(services_handle,
             "** TEST %s error_code=%s\n",
             p_test_name,
             SERVICES_error_to_string(error_code));

  return error_code;
}

/**
 * @brief BOOT TOC A32
 *
 * @param p_test_name
 * @param services_handle
 * @return
 */
static uint32_t test_services_boot_toc_a32(char *p_test_name,
                                           uint32_t services_handle)
{
  const uint32_t BL_ERROR_INVALID_TOC_ENTRY_ID = 0x21; /*boot_loader.h*/
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code = 0;
  uint8_t entry_id[8];

  memset(entry_id, 0x0, sizeof(entry_id));
  strcpy((char *)entry_id, "A32_3");

  error_code = SERVICES_boot_process_toc_entry(services_handle,
                                               entry_id,
                                               &service_error_code);

  TEST_print(services_handle,
             "** TEST %s error_code=%s service_resp=0x%08X\n",
             p_test_name,
             SERVICES_error_to_string(error_code),
             service_error_code);
  if (service_error_code == BL_ERROR_INVALID_TOC_ENTRY_ID)
  {
    TEST_print(services_handle,
               "    -> Function returned BL_ERROR_INVALID_TOC_ENTRY_ID as expected for %s\n",
               entry_id);
  }
  else if (service_error_code != 0)
  {
    TEST_print(services_handle,
               "    -> Function returned unexpected error code 0x%08X\n",
               service_error_code);
  }

  return error_code;
}

/**
 * @brief BOOT CPU
 *
 * @param p_test_name
 * @param services_handle
 * @return
 */
static uint32_t test_services_boot_cpu(char *p_test_name,
                                       uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code = 0;
  uint32_t cpu_id = EXTSYS_0;
  uint32_t address = 0;

  error_code = SERVICES_boot_cpu(services_handle,
                                 cpu_id,
                                 address,
                                 &service_error_code);

  PRINT_TEST_RESULT;

  if (service_error_code == SERVICE_FAIL)
  {
    TEST_print(services_handle,
               "    -> Function returned SERVICE_FAIL as expected for cpu_id=%d\n",
               (int)cpu_id);
  }
  else if (service_error_code != 0)
  {
    TEST_print(services_handle,
               "    -> Function returned unexpected error code 0x%08X\n",
               service_error_code);
  }

  return error_code;
}

/**
 * @brief BOOT RESET CPU
 *
 * @param p_test_name
 * @param services_handle
 * @return
 */
static uint32_t test_services_boot_reset_cpu(char *p_test_name,
                                             uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code = 0;
  uint32_t cpu_id = EXTSYS_0;

  error_code = SERVICES_boot_reset_cpu(services_handle,
                                       cpu_id,
                                       &service_error_code);

  PRINT_TEST_RESULT;

  if (service_error_code == SERVICE_FAIL)
  {
    TEST_print(services_handle,
               "    -> Function returned SERVICE_FAIL as expected for cpu_id=%d\n",
               (int)cpu_id);
  }
  else if (service_error_code != 0)
  {
    TEST_print(services_handle,
               "    -> Function returned unexpected error code 0x%08X\n",
               service_error_code);
  }

  return error_code;
}

/**
 * @brief BOOT TOC ExtSys0
 *
 * @param p_test_name
 * @param services_handle
 * @return
 */
static uint32_t test_services_boot_release_extsys0(char *p_test_name,
                                                   uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  error_code = SERVICES_boot_release_cpu(services_handle,
                                         EXTSYS_0,
                                         &service_error_code);

  PRINT_TEST_RESULT;

  return error_code;
}

/**
 * @brief MBEDTLS AES
 *
 * @param p_test_name
 * @param services_handle
 * @return
 */
static uint32_t test_services_mbedtls_aes(char *p_test_name,
                                          uint32_t services_handle)
{
  /*
   * In C++, const really means const and can be used to declare static array sizes
   * In C, it is not a constant variable expression
   */
#define KEY_SIZE 256
#define AES_IV_SIZE 16
#define AES_BLOCK_SIZE 16

  /* https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Standards-and-Guidelines/documents/examples/AES_OFB.pdf */
  static const uint8_t IV[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
  static const uint8_t PLAIN[] = { 0x6B, 0xC1, 0xBE, 0xE2, 0x2E, 0x40, 0x9F,
                                   0x96, 0xE9, 0x3D, 0x7E, 0x11, 0x73, 0x93,
                                   0x17, 0x2A };
  static const uint8_t KEY[] = { 0x60, 0x3D, 0xEB, 0x10, 0x15, 0xCA, 0x71, 0xBE,
                                 0x2B, 0x73, 0xAE, 0xF0, 0x85, 0x7D, 0x77, 0x81,
                                 0x1F, 0x35, 0x2C, 0x07, 0x3B, 0x61, 0x08, 0xD7,
                                 0x2D, 0x98, 0x10, 0xA3, 0x09, 0x14, 0xDF, 0xF4 };
  static const uint8_t CYPHER[] = { 0xDC, 0x7E, 0x84, 0xBF, 0xDA, 0x79, 0x16,
                                    0x4B, 0x7E, 0xCD, 0x84, 0x86, 0x98, 0x5D,
                                    0x38, 0x60 };

  uint8_t key[KEY_SIZE / 8];
  uint8_t iv[AES_IV_SIZE];
  uint8_t buf[AES_BLOCK_SIZE];
  
  uint32_t aes_ctx[24]={0};

  // const char* TEST_NAME = "AES-OFB-256";

  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  /*currently unused*/
  (void)(p_test_name);

  // Initialize aes engine
  SERVICES_cryptocell_mbedtls_aes_init(services_handle, &service_error_code,
                                       (uint32_t)aes_ctx);

  memcpy(key, KEY, sizeof(key));
  memcpy(buf, PLAIN, sizeof(buf));
  memcpy(iv, IV, sizeof(iv));

  TEST_print(services_handle,
             "** TEST SERVICES_cryptocell_mbedtls_aes_init    error_code=%s service_resp=0x%08X\n",
             SERVICES_error_to_string(error_code),
             service_error_code);

  // encrypt
  // set key into context
  SERVICES_cryptocell_mbedtls_aes_set_key(services_handle, 
                                          &service_error_code, 
                                          (uint32_t)aes_ctx, 
                                          (uint32_t)key, 
                                          KEY_SIZE,
                                          MBEDTLS_OP_ENCRYPT);
  TEST_print(services_handle,
             "** TEST SERVICES_cryptocell_mbedtls_aes_set_key error_code=%s service_resp=0x%08X\n",
             SERVICES_error_to_string(error_code),
             service_error_code);

  // perform cryptographic operation
  //mbedtls_aes_crypt_ofb(&ctx, sizeof(buf), 0, iv, buf, buf);
  SERVICES_cryptocell_mbedtls_aes_crypt(services_handle, 
                                        &service_error_code, 
                                        (uint32_t)aes_ctx,
                                        MBEDTLS_AES_CRYPT_OFB, 
                                        0, 
                                        sizeof(buf), 
                                        (uint32_t)iv, 
                                        (uint32_t)buf, 
                                        (uint32_t)buf);

  memcpy(buf, CYPHER, sizeof(buf));
  memcpy(iv, IV, sizeof(iv));
  TEST_print(services_handle,
             "** TEST SERVICES_cryptocell_mbedtls_aes_crypt   error_code=%s service_resp=0x%08X\n",
             SERVICES_error_to_string(error_code),
             service_error_code);
  // decrypt
  // set key into context
  //mbedtls_aes_setkey_dec(&ctx, key, KEY_SIZE);
  SERVICES_cryptocell_mbedtls_aes_set_key(services_handle, 
                                          &service_error_code, 
                                          (uint32_t)aes_ctx, 
                                          (uint32_t)key, 
                                          KEY_SIZE,
                                          MBEDTLS_OP_DECRYPT);
  TEST_print(services_handle,
             "** TEST SERVICES_cryptocell_mbedtls_aes_set_key error_code=%s service_resp=0x%08X\n",
             SERVICES_error_to_string(error_code),
             service_error_code);

  // perform cryptographic operation
  //mbedtls_aes_crypt_ofb(&ctx, sizeof(buf), 0, iv, buf, buf);
  SERVICES_cryptocell_mbedtls_aes_crypt(services_handle, 
                                        &service_error_code, 
                                        (uint32_t)aes_ctx,
                                        MBEDTLS_AES_CRYPT_OFB, 
                                        0, 
                                        sizeof(buf), 
                                        (uint32_t)iv, 
                                        (uint32_t)buf, 
                                        (uint32_t)buf);
  TEST_print(services_handle,
             "** TEST SERVICES_cryptocell_mbedtls_aes_crypt   error_code=%s service_resp=0x%08X\n",
             SERVICES_error_to_string(error_code),
             service_error_code);

  return error_code;
}

static uint32_t test_services_mbedtls_sha(char *p_test_name,
                                          uint32_t services_handle)
{
  (void)(p_test_name);

#define SHA256_BYTES         32  // 256 bits
#define SHA256_PRINT         64  // 256 bits is 64 characters

  char * test_payload = "SHA 256";
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t sha_ctx[60]={0};
  uint8_t sha256sum[SHA256_BYTES];

  SERVICES_cryptocell_mbedtls_sha_starts(services_handle,
                                         &error_code,
                                         (uint32_t)sha_ctx,
                                         MBEDTLS_HASH_SHA256);
  SERVICES_cryptocell_mbedtls_sha_update(services_handle,
                                         &error_code,
                                         (uint32_t)sha_ctx,
                                         MBEDTLS_HASH_SHA256,
                                         (uint32_t)test_payload,
                                         strlen(test_payload));
  SERVICES_cryptocell_mbedtls_sha_finish(services_handle,
                                         &error_code,
                                         (uint32_t)sha_ctx,
                                         MBEDTLS_HASH_SHA256,
                                         (uint32_t)sha256sum);

  uint8_t buf[10] = {0};
  uint8_t print_buf[SHA256_PRINT] = {0};
  for (uint32_t i = 0; i < SHA256_BYTES; i++)
  {
    sprintf((void *)buf, "%02X", sha256sum[i]);
    strcat((void *)print_buf, (void *)buf);
  }
  TEST_print(services_handle,
             "** TEST SERVICES_cryptocell_mbedtls_sha   service_resp=0x%08X\n",
             error_code);

  TEST_print(services_handle, "SHA256 sum: %s\n", print_buf);

  return 0;
}

/**
 * @brief Bound check tests - UART
 *
 * @param p_test_name
 * @param services_handle
 * @return
 */
static uint32_t test_services_bounds(char *p_test_name,
                                     uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint8_t buffer[PRINT_BUFFER_SIZE] = { 0 };

  error_code = SERVICES_uart_write(services_handle,
                                   0,
                                   (uint8_t*)&buffer[0]);
  TEST_print(services_handle,
             "** TEST %s error_code=%s service_resp=0x%08X\n",
             p_test_name,
             SERVICES_error_to_string(error_code));

  error_code = SERVICES_uart_write(services_handle,
                                   PRINT_BUFFER_SIZE+1,
                                   (uint8_t*)&buffer[0]);
  TEST_print(services_handle,
             "** TEST %s error_code=%s service_resp=0x%08X\n",
             p_test_name,
             SERVICES_error_to_string(error_code));

  error_code = SERVICES_uart_write(services_handle,
                                     20,
                                     (uint8_t*)NULL);

  TEST_print(services_handle,
               "** TEST %s error_code=%s service_resp=0x%08X\n",
               p_test_name,
               SERVICES_error_to_string(error_code));

  return error_code;
}

/**
 * @brief OSPI Key write test
 *
 * @param p_test_name
 * @param services_handle
 * @return
 */
static uint32_t test_services_ospi_write_key(char *p_test_name,
                                             uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code = 0;
  uint8_t key[16] = {0x0, 0x1, 0x2, 0x3, 0x4,
                     0x5, 0x6, 0x7, 0x8, 0x9,
                     0xA, 0xB, 0xC, 0xD, 0xE,
                     0xF};
  uint32_t commands[4] = {
      OSPI_WRITE_OTP_KEY_OSPI0, OSPI_WRITE_OTP_KEY_OSPI1,
      OSPI_WRITE_EXTERNAL_KEY_OSPI0, OSPI_WRITE_EXTERNAL_KEY_OSPI1
  };

  for (int i = 0; i < 4; i++)
  {
    error_code = SERVICES_application_ospi_write_key(services_handle,
                                                     commands[i],
                                                     key, 
                                                     &service_error_code);
    TEST_print(services_handle,
               "** TEST %s command=0x%X error_code=%s service_resp=0x%08X\n",
               p_test_name,
               commands[i],
               SERVICES_error_to_string(error_code),
               service_error_code);
  }

  return error_code;
}

/**
 * @brief Select OSC Source service test
 */
static uint32_t test_services_select_osc_source(char *p_test_name, uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  error_code = SERVICES_clocks_select_osc_source(services_handle,
                         /*oscillator_source_t*/ OSCILLATOR_SOURCE_XTAL,
                         /*oscillator_target_t*/ OSCILLATOR_TARGET_PERIPH_CLOCKS,
                                                 &service_error_code);
  PRINT_TEST_RESULT;

  return error_code;
}

/**
 * @brief Select PLL Source service test
 */
static uint32_t test_services_select_pll_source(char *p_test_name, uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  error_code = SERVICES_clocks_select_pll_source(services_handle,
                                /*pll_source_t*/ PLL_SOURCE_PLL,
                                /*pll_target_t*/ PLL_TARGET_ES0,
                                                 &service_error_code);
  PRINT_TEST_RESULT;

  return error_code;
}

/**
 * @brief Service Enable service test
 */
static uint32_t test_services_enable_clock(char *p_test_name, uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  /*
   * Consider looping from CLKEN_SYSPLL to CLKEN_HFOSC
   */
  error_code = SERVICES_clocks_enable_clock(services_handle,
                         /*clock_enable_t*/ CLKEN_ES0,
                         /*bool enable   */ true,
                                            &service_error_code);
  PRINT_TEST_RESULT;

  return error_code;
}

/**
 * @brief Set ES0 Frequency service test
 */
static uint32_t test_services_set_ES0_frequency(char *p_test_name, uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  error_code = SERVICES_clocks_set_ES0_frequency(services_handle,
                           /*clock_frequency_t*/ CLOCK_FREQUENCY_400MHZ,
                                                 &service_error_code);
  PRINT_TEST_RESULT;

  return error_code;
}

/**
 * @brief Set ES1 Frequency service test
 */
static uint32_t test_services_set_ES1_frequency(char *p_test_name, uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  error_code = SERVICES_clocks_set_ES1_frequency(services_handle,
                           /*clock_frequency_t*/ CLOCK_FREQUENCY_160MHZ,
                                                 &service_error_code);
  PRINT_TEST_RESULT;

  return error_code;
}

/**
 * @brief Select A32 Source service test
 */
static uint32_t test_services_select_a32_source(char *p_test_name, uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  error_code = SERVICES_clocks_select_a32_source(services_handle,
                                /*a32_source_t*/ A32_SYSPLL,
                                                 &service_error_code);
  PRINT_TEST_RESULT;

  return error_code;
}

/**
 * @brief Select ACLK Source service test
 */
static uint32_t test_services_select_aclk_source(char *p_test_name, uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  error_code = SERVICES_clocks_select_aclk_source(services_handle,
                                /*aclk_source_t*/ ACLK_SYSPLL,
                                                  &service_error_code);
  PRINT_TEST_RESULT;

  return error_code;
}

/**
 * @brief Set Clock Divider service test
 */
static uint32_t test_services_set_divider(char *p_test_name, uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  error_code = SERVICES_clocks_set_divider(services_handle,
                                           DIVIDER_CPUPLL,
                                           0x0, // divide by 0
                                           &service_error_code);
  PRINT_TEST_RESULT;

  return error_code;
}

/**
 * @brief PLL Initialize service test
 */
static uint32_t test_services_pll_initialize(char *p_test_name, uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  error_code = SERVICES_pll_initialize(services_handle, &service_error_code);
  PRINT_TEST_RESULT;

  return 0;
}

/**
 * @brief PLL De-Init service test
 */
static uint32_t test_services_pll_deinit(char *p_test_name, uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  error_code = SERVICES_pll_initialize(services_handle, &service_error_code);
  PRINT_TEST_RESULT;

  return 0;
}

/**
 * @brief XTAL Start service test
 */
static uint32_t test_services_xtal_start(char *p_test_name, uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  error_code = SERVICES_pll_xtal_start(services_handle, true, true, 15000, &service_error_code);
  PRINT_TEST_RESULT;

  return 0;
}

/**
 * @brief XTAL Stop service test
 */
static uint32_t test_services_xtal_stop(char *p_test_name, uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  error_code = SERVICES_pll_xtal_stop(services_handle, &service_error_code);
  PRINT_TEST_RESULT;

  return 0;
}

/**
 * @brief XTAL Is Started service test
 */
static uint32_t test_services_xtal_is_started(char *p_test_name, uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  bool is_started;
  error_code = SERVICES_pll_xtal_is_started(services_handle,
                                            &is_started,
                                            &service_error_code);

  TEST_print(services_handle,
             "** TEST %s: started=%d \terror_code=%s service_resp=0x%08X\n",
             p_test_name,
             is_started,
             SERVICES_error_to_string(error_code),
             service_error_code);

  return error_code;
}

/**
 * @brief PLL Start test
 */
static uint32_t test_services_clkpll_start(char *p_test_name, uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  error_code = SERVICES_pll_clkpll_start(services_handle, true, 15000, &service_error_code);
  PRINT_TEST_RESULT;

  return 0;
}

/**
 * @brief PLL Stop test
 */
static uint32_t test_services_clkpll_stop(char *p_test_name, uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  error_code = SERVICES_pll_clkpll_stop(services_handle, &service_error_code);
  PRINT_TEST_RESULT;

  return 0;
}

/**
 * @brief PLL Is Locked test
 */
static uint32_t test_services_clkpll_is_locked(char *p_test_name, uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  bool is_locked;
  error_code = SERVICES_pll_clkpll_is_locked(services_handle,
                                             &is_locked,
                                             &service_error_code);

  TEST_print(services_handle,
             "** TEST %s: locked=%d \terror_code=%s service_resp=0x%08X\n",
             p_test_name,
             is_locked,
             SERVICES_error_to_string(error_code),
             service_error_code);

  return error_code;
}

/**
 * @brief Test sequence for PLL and XTAL services
 */
static uint32_t test_services_pll_xtal(char *p_test_name, uint32_t services_handle)
{
  uint32_t service_error_code;

  TEST_print(services_handle, "** TEST %s\n", p_test_name);

  // Switch from PLL to OSC clocks
  uint32_t pll_source = PLL_SOURCE_OSC;
  SERVICES_clocks_select_pll_source(services_handle, pll_source, PLL_TARGET_UART,        &service_error_code);
  SERVICES_clocks_select_pll_source(services_handle, pll_source, PLL_TARGET_ES0,         &service_error_code);
  SERVICES_clocks_select_pll_source(services_handle, pll_source, PLL_TARGET_ES1,         &service_error_code);
  TEST_print(services_handle, "Switching from PLL to RC: %d\n", service_error_code);
  SERVICES_clocks_select_pll_source(services_handle, pll_source, PLL_TARGET_SYSREFCLK,   &service_error_code);
  SERVICES_clocks_select_pll_source(services_handle, pll_source, PLL_TARGET_SYSCLK,      &service_error_code);
  SERVICES_clocks_select_pll_source(services_handle, pll_source, PLL_TARGET_SECENC,      &service_error_code);
  SERVICES_clocks_select_pll_source(services_handle, pll_source, PLL_TARGET_PD4_SRAM,    &service_error_code);

  // Switch from XTAL to RC
  SERVICES_clocks_select_osc_source(services_handle, OSCILLATOR_SOURCE_RC, OSCILLATOR_TARGET_SYS_CLOCKS, &service_error_code);
  SERVICES_clocks_select_osc_source(services_handle, OSCILLATOR_SOURCE_RC, OSCILLATOR_TARGET_PERIPH_CLOCKS, &service_error_code);

  // Stop PLL and XTAL
  SERVICES_pll_clkpll_stop(services_handle, &service_error_code);
  SERVICES_pll_xtal_stop(services_handle, &service_error_code);

  // Start XTAL and PLL
  SERVICES_pll_xtal_start(services_handle, true, true, 15000, &service_error_code);
  SERVICES_pll_clkpll_start(services_handle, true, 15000, &service_error_code);

  // switch from OSC to PLL clocks
  pll_source = PLL_SOURCE_PLL;
  SERVICES_clocks_select_pll_source(services_handle, pll_source, PLL_TARGET_SYSREFCLK,   &service_error_code);
  SERVICES_clocks_select_pll_source(services_handle, pll_source, PLL_TARGET_SYSCLK,      &service_error_code);
  SERVICES_clocks_select_pll_source(services_handle, pll_source, PLL_TARGET_SECENC,      &service_error_code);
  TEST_print(services_handle, "Switching from RC to PLL: %d\n", service_error_code);
  SERVICES_clocks_select_pll_source(services_handle, pll_source, PLL_TARGET_UART,        &service_error_code);
  SERVICES_clocks_select_pll_source(services_handle, pll_source, PLL_TARGET_ES0,         &service_error_code);
  SERVICES_clocks_select_pll_source(services_handle, pll_source, PLL_TARGET_ES1,         &service_error_code);
  SERVICES_clocks_select_pll_source(services_handle, pll_source, PLL_TARGET_PD4_SRAM,    &service_error_code);

  // Recover the device
  SERVICES_pll_initialize(services_handle, &service_error_code);

  // Change the frequencies of ES0 and ES1
  SERVICES_clocks_set_ES0_frequency(services_handle,  CLOCK_FREQUENCY_200MHZ, &service_error_code);
  SERVICES_clocks_set_ES1_frequency(services_handle,  CLOCK_FREQUENCY_80MHZ, &service_error_code);

  return SERVICES_REQ_SUCCESS;
}

/**
 * @brief BOOT RESET SOC
 *
 * @param p_test_name
 * @param services_handle
 * @return
 */
static uint32_t test_services_boot_reset_soc(char *p_test_name,
                                             uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;

  uint32_t counter = 0;
#if 0
  typedef struct { uint32_t D[4]; } CTR;
  CTR *pCounter;

  /*keep track of how many times we have booted*/
  /*
   * TODO: identify register or memory that is retained between cold boots
   *       for now we use MRAM
   */
  pCounter = (CTR *)(0x80400000); //MRAM but before the ATOC
  counter  = pCounter->D[0];
  pCounter->D[0] = counter + 1;
  pCounter->D[1] = counter + 1;
  pCounter->D[2] = counter + 1;
  pCounter->D[3] = counter + 1;
  TEST_print(services_handle, "[%p] counter = %d, NEW pCounter->D[0] = %d)\n", pCounter, (int)counter, (int)pCounter->D[0]);
#endif
  if ((counter < 6) || (counter == 8))
  {
    TEST_print(services_handle,
               "*** TEST %s counter = %d - reset SOC\n",
               p_test_name,
               counter);
    error_code = SERVICES_boot_reset_soc(services_handle);
    /*NOT REACHED*/
  }
  else
  {
    TEST_print(services_handle,
               "*** TEST %s counter = %d - SKIP resetting SOC\n",
               p_test_name,
               counter);
  }

  return error_code;
}

/**
 * @brief Test booting a CPU core via the low level APIs
 */
static uint32_t test_services_cpu_boot_sequence(
    char *p_test_name, uint32_t services_handle)
{

  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  error_code = SERVICES_boot_set_vtor(services_handle,
                                      EXTSYS_0,
                                      0x80100000,
                                      &service_error_code);

  if (error_code == SERVICES_REQ_SUCCESS && service_error_code == SERVICE_SUCCESS)
  {
    error_code = SERVICES_boot_reset_cpu(services_handle,
                                         EXTSYS_0,
                                         &service_error_code);
  }

  if (error_code == SERVICES_REQ_SUCCESS && service_error_code == SERVICE_SUCCESS)
  {
    error_code = SERVICES_boot_release_cpu(services_handle,
                                           EXTSYS_0,
                                           &service_error_code);
  }

  PRINT_TEST_RESULT;

  return error_code;
}

static uint32_t test_services_dcdc_voltage(char *p_test_name,
                                           uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  // same values as during cold boot
  uint32_t dcdc_vout_sel  = DCDC_VOUT_0825;
  uint32_t dcdc_vout_trim = 4;

  error_code = SERVICES_power_dcdc_voltage_control(services_handle,
                                                   dcdc_vout_sel,
                                                   dcdc_vout_trim,
                                                   &service_error_code);

  TEST_print(services_handle,
              "** TEST %s error_code=%s service_resp=0x%08X\n",
              p_test_name,
              SERVICES_error_to_string(error_code),
              service_error_code);

  TEST_print(services_handle, "dcdc voltage: dcdc_vout_sel=0x%x; "
      "dcdc_vout_trim=0x%x \n",
             dcdc_vout_sel, dcdc_vout_trim);

  return error_code;
}

static uint32_t test_services_ldo_voltage(char *p_test_name,
                                         uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  uint32_t ret_ldo_voltage  = 0x8;
  uint32_t aon_ldo_voltage  = 0xF;

  error_code = SERVICES_power_ldo_voltage_control(services_handle,
                                                  ret_ldo_voltage,
                                                  aon_ldo_voltage,
                                                  &service_error_code);

  TEST_print(services_handle,
              "** TEST %s error_code=%s service_resp=0x%08X\n",
              p_test_name,
              SERVICES_error_to_string(error_code),
              service_error_code);

  TEST_print(services_handle, "ldo voltage: ret_ldo_voltage=0x%x; "
      "aon_ldo_voltage=0x%x \n",
      ret_ldo_voltage, aon_ldo_voltage);

  return error_code;
}

static uint32_t test_services_get_bus_frequencies(char *p_test_name, uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  uint32_t frequency;
  error_code = SERVICES_clocks_get_refclk_frequency(services_handle,
                                                    &frequency,
                                                    &service_error_code);

  TEST_print(services_handle,
              "** TEST %s error_code=%s service_resp=0x%08X\n",
              p_test_name,
              SERVICES_error_to_string(error_code),
              service_error_code);
  TEST_print(services_handle, "REFCLK frequency %d\n", frequency);

  error_code = SERVICES_clocks_get_apb_frequency(services_handle,
                                                 &frequency,
                                                 &service_error_code);
  TEST_print(services_handle, "APB frequency %d\n", frequency);

  return error_code;
}

/**
 * @brief Generate 24/40 bit unique extensions for EUI-48 and EUI-64
 */
static uint32_t test_services_get_eui(char *p_test_name, uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  uint8_t eui[5];
  error_code = SERVICES_system_get_eui_extension(services_handle,
                                                 true,
                                                 eui,
                                                 &service_error_code);
  TEST_print(services_handle,
              "** TEST %s error_code=%s service_resp=0x%08X\n",
              p_test_name,
              SERVICES_error_to_string(error_code),
              service_error_code);

  TEST_print(services_handle, "EUI-48 extension: %02X-%02X-%02X\n",
                              eui[0], eui[1], eui[2]);

  SERVICES_system_get_eui_extension(services_handle, false, eui, &service_error_code);
  TEST_print(services_handle, "EUI-64 extension: %02X-%02X-%02X-%02X-%02X\n",
                              eui[0], eui[1], eui[2], eui[3], eui[4]);

  return error_code;
}

/**
 * @brief Test the BOR_EN coontrol
 */
static uint32_t test_services_bor_en(char *p_test_name, uint32_t services_handle)
{
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  uint32_t service_error_code;

  uint32_t bor_en = 0;
  error_code = SERVICES_power_setting_get(services_handle,
                                          POWER_SETTING_BOR_EN,
                                          &bor_en,
                                          &service_error_code);
  PRINT_TEST_RESULT;

  TEST_print(services_handle, "Current BOR_EN: %d\n", bor_en);
  bor_en = !bor_en;
  error_code = SERVICES_power_setting_configure(services_handle,
                                                POWER_SETTING_BOR_EN,
                                                bor_en,
                                                &service_error_code);
  PRINT_TEST_RESULT;

  error_code = SERVICES_power_setting_get(services_handle,
                                          POWER_SETTING_BOR_EN,
                                          &bor_en,
                                          &service_error_code);

  PRINT_TEST_RESULT;
  TEST_print(services_handle, "New BOR_EN: %d\n", bor_en);

  // restore the original BOR_EN
  TEST_print(services_handle, "Restore original BOR_EN\n");
  bor_en = !bor_en;
  error_code = SERVICES_power_setting_configure(services_handle,
                                                POWER_SETTING_BOR_EN,
                                                bor_en,
                                                &service_error_code);
  PRINT_TEST_RESULT;

  return error_code;
}

/**
 * @fn    void setup_tests(void)
 * @brief enable/disable individual tests based on configuration
 */
static void setup_tests(void)
{
#if SANITY_TESTS_ENABLE == 1
  uint32_t number_of_tests = sizeof(s_tests)/sizeof(services_test_t);
  for (uint32_t test_idx = 0; test_idx < number_of_tests; test_idx++)
  {
    s_tests[test_idx].enabled = true;
  }

#if A32_BOOT_WORKAROUND == 1
  // these tests crash the current B0 device
  s_tests[21].enabled = false; /*test_services_boot_toc_a32*/
  s_tests[22].enabled = false; /*test_services_boot_cpu*/
  s_tests[23].enabled = false; /*test_services_boot_reset_cpu*/
#endif // A32_BOOT_WORKAROUND
  // disable tests not part of the 'sanity tests' group
  s_tests[44].enabled = false; /*test_services_pll_xtal*/
#endif // SANITY_TESTS_ENABLE

#if PLL_XTAL_TESTS_ENABLE == 1
  s_tests[0].enabled  = true; /*test_services_heartbeat*/
  s_tests[44].enabled = true; /*test_services_pll_xtal*/
#else
#if PLATFORM_TYPE == FPGA
  /*PLL tests will not work on an FPGA*/
  s_tests[30].enabled = false; /*test_services_pll_deinit*/
  s_tests[31].enabled = false; /*test_services_pll_initialize*/
  s_tests[33].enabled = false; /*test_services_clkpll_is_locked*/
  s_tests[38].enabled = false; /*test_services_select_pll_source*/
#endif
#endif

  s_tests[43].enabled = false; /*test_services_boot_reset_soc*/

  /*these static functions were not being called*/
  s_tests[45].enabled = false; /*test_services_clkpll_start*/
  s_tests[46].enabled = false; /*test_services_clkpll_stop*/
  s_tests[47].enabled = false; /*test_services_xtal_start*/
  s_tests[48].enabled = false; /*test_services_xtal_stop*/

#if CPU_BOOT_SEQUENCE_TEST_ENABLE == 1
  s_tests[49].enabled = true; /*test_services_cpu_boot_sequence*/
#else
  s_tests[49].enabled = false; /*test_services_cpu_boot_sequence*/
#endif
}

/**
 * @fn    void SERVICES_test(uint32_t services_handle)
 * @brief test harness for supported SERVICE APIS
 */
void SERVICES_test_guts(uint32_t services_handle)
{
  /* A32 calls SERVICES_test_guts() directly from main() */
  setup_tests();

  /* show services version */
  TEST_print(services_handle, "[%s] SERVICES version %s\n",
             CPU_STRING, SERVICES_version());

  uint32_t number_of_tests = sizeof(s_tests)/sizeof(services_test_t);
  //uint32_t number_of_tests = 3;
  for (uint32_t test_idx = 0; test_idx < number_of_tests; test_idx++)
  {
    services_test_t *p_test;
    uint32_t error_code;
#if RANDOMIZER_FEATURE == 1
    uint32_t random_test;
    srand(time());
    random_test = rand()%(NUMBER_OF_TESTS);
    p_test = (services_test_t *)&s_tests[random_test];
#else
    p_test = (services_test_t *)&s_tests[test_idx];
#endif
    if (s_tests[test_idx].enabled)
    {
      error_code = p_test->test_fn(p_test->test_name,services_handle);
    }
    else
    {
      TEST_print(services_handle, "[%s] SKIP: %s\n", CPU_STRING, p_test->test_name);
    }
    (void)error_code;
  }
}

/**
 * @fn    void SERVICES_test(uint32_t services_handle)
 * @brief test harness for supported SERVICE APIS
 */
void SERVICES_test(uint32_t services_handle)
{
  TEST_init(services_handle);

  SERVICES_test_guts(services_handle);
  test_crypto(services_handle);
}
