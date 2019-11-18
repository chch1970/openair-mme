/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the Apache License, Version 2.0  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */



/*! \file mce_app_mbms_service_context.h
 *  \brief MCE applicative layer
 *  \author Dincer Beken
 *  \date 2019
 *  \version 1.0
 *  \email: dbeken@blackned.de
 *  @defgroup _mce_app_impl_ MCE applicative layer
 *  @ingroup _ref_implementation_
 *  @{
 */

#ifndef FILE_MCE_APP_MBMS_SERVICE_CONTEXT_SEEN
#define FILE_MCE_APP_MBMS_SERVICE_CONTEXT_SEEN
#include <stdint.h>
#include <inttypes.h>   /* For sscanf formats */
#include <time.h>       /* to provide time_t */

#include "tree.h"
#include "queue.h"
#include "hashtable.h"
#include "obj_hashtable.h"
#include "bstrlib.h"
#include "common_types.h"
#include "mce_app_messages_types.h"
#include "mme_app_bearer_context.h"
#include "sm_messages_types.h"
#include "m3ap_messages_types.h"
#include "mme_app_procedures.h"
//#include "security_types.h"

#define MBMS_SERVICE_ID_DIGITS 6

typedef struct {
  uint32_t length;
  char data[MBMS_SERVICE_ID_DIGITS];
} mce_app_mbms_service_id_t;

typedef int ( *mce_app_mbms_service_callback_t) (void*);

#define MBMS_SERVICE_ID_FORMAT "s"
#define MBMS_SERVICE_ID_DATA(MCE_APP_MBMS_SERVICE_ID) (MCE_APP_MBMS_SERVICE_ID.data)

/* Convert the MBMS Service ID contained by a char string NULL terminated to uint64_t */
bool mce_app_is_mbms_service_id_empty(mce_app_mbms_service_id_t const * mbms_service_id);
bool mce_app_mbms_service_id_compare(mce_app_mbms_service_id_t const * mbms_service_id_a, mce_app_mbms_service_id_t const * mbms_service_id_b);
void mce_app_copy_mbms_service_id(mce_app_mbms_service_id_t * mbms_service_id_dst, mce_app_mbms_service_id_t const * mbms_service_id_src);

/*
 * Timer identifier returned when in inactive state (timer is stopped or has
 * failed to be started)
 */
#define MCE_APP_TIMER_INACTIVE_ID   (-1)
#define MCE_APP_DELTA_T3412_REACHABILITY_TIMER 4 // in minutes
#define MCE_APP_DELTA_REACHABILITY_IMPLICIT_DETACH_TIMER 0 // in minutes
#define MCE_APP_INITIAL_CONTEXT_SETUP_RSP_TIMER_VALUE 2 // In seconds

// todo: #define MCE_APP_INITIAL_CONTEXT_SETUP_RSP_TIMER_VALUE 2 // In seconds
/* Timer structure */
//-----------------
struct mce_app_timer_t {
  long id;         /* The timer identifier                 */
  long sec;       /* The timer interval value in seconds  */
};

/** @struct mbms_service_s
 *  @brief Useful parameters to know in MCE application layer. They are set
 * according to 3GPP TS.29.486
 */
//-----------------
typedef struct mbms_service_s {
  struct {
	  pthread_mutex_t recmutex;  // mutex on the ue_context_t
	  struct {
		  /* TODO: add DRX parameter */
		  // UE Specific DRX Parameters   // UE specific DRX parameters for A/Gb mode, Iu mode and S1-mode
		  /** TMGI and MBMS Service Area of the MBMS Service - Key Identifiers. */
		  tmgi_t		         			tmgi;
		  mbms_service_area_id_t 			mbms_service_area_id;
		  /** MCE TEID for Sm. */
		  teid_t                 			mme_teid_sm;                // needed to get the MBMS Service from Sm messages
		  teid_t                 			mbms_teid_sm;
		  /** Flow-Id of the MBMS Bearer service. */
		  uint16_t               			mbms_flow_id;
		  /**
		   * MBMS Bearer Context created for all eNBs of the MBMS Service Area for this MBMS Service.
		   * It is bound to the TMGI and flow ID combination.
		   */
		  struct mbms_bearer_context_s  	mbms_bc;

		  /** MBMS Peer Information. */
		  union {
		    struct sockaddr_in            mbms_peer_ipv4;             ///< MME ipv4 address for S-GW or S-GW ipv4 address for MME.
		    struct sockaddr_in6           mbms_peer_ipv6;             ///< MME ipv4 address for S-GW or S-GW ipv4 address for MME.
		  }mbms_peer_ip;
	  }fields;
  }privates;

  mme_app_mbms_proc_t					 *mbms_procedure;			  ///< Allowing a single MBMS procedure in the MBMS-Service.

  /** Entries for MBMS Service Pool . */
  STAILQ_ENTRY (mbms_service_s)		entries;
} mbms_service_t;

/** @struct mbsfn_area_context_s
 *  @brief Useful parameters to know in MCE application layer. They are set
 * according to 3GPP TS.36.443
 */
//-----------------
typedef struct mbsfn_area_context_s {
  struct {
	  pthread_mutex_t recmutex;  // mutex on the ue_context_t
	  struct {
		  mbsfn_area_t 	mbsfn_area;
	  }fields;
  }privates;
  // todo: no procedure but a timer!
  // mme_app_mbms_proc_t					 *mbms_procedure;			  ///< Allowing a single MBMS procedure in the MBMS-Service.
  /** Entries for MBSFN Area Pool . */
  STAILQ_ENTRY (mbsfn_area_context_s)		entries;
} mbsfn_area_context_t;

//-----------------
typedef struct mce_mbms_services_s {
  uint32_t                 nb_mbms_service_managed;
  uint32_t                 nb_mbms_service_since_last_stat;
  hash_table_ts_t 		  	*mbms_service_index_mbms_service_htbl;    // data is mbms_service_t
  hash_table_uint64_ts_t  *tunsm_mbms_service_htbl;					// data is mbms_service_index_t
} mce_mbms_services_t;

//-----------------
typedef struct mce_mbsfn_areas_s {
  uint32_t                 nb_mbsfn_area_managed;
  uint32_t                 nb_mbsfn_are_since_last_stat;
  hash_table_ts_t 		  	*mbsfn_area_id_mbsfn_area_htbl;    	// data is mbsfn_area_t
} mce_mbsfn_areas_t;

/** \brief Retrieve an MBMS service by selecting the given MBMS Service Area And TMGI.
 * \param tmgi TMGI to find in MBMS Service map
 * @returns an MBMS Service context matching the TMGI and MBMS Service Area or NULL if the context doesn't exists
 **/
struct mbms_service_s *mce_mbms_service_exists_tmgi(mce_mbms_services_t * const mce_mbms_services, const tmgi_t * const tmgi, const mbms_service_area_id_t mbms_service_area_id);

/** \brief Retrieve an MBMS service by selecting the given MBMS Service Index. Used mainly for expired Sm procedures.
 * \param mbms service index
 * @returns an MBMS Service context matching the MBMS Service Index or NULL if the context doesn't exists
 **/
struct mbms_service_s *mce_mbms_service_exists_mbms_service_index(mce_mbms_services_t * const mce_mbms_services_p, const mbms_service_index_t mbms_service_index);

/** \brief Retrieve an MBMS service by selecting the given SM TEID.
 * \param teid MME-SM TEID to find in MBMS Service map
 * @returns an MBMS Service context matching the MME SM TEID or NULL if the context doesn't exists
 **/
struct mbms_service_s *mce_mbms_service_exists_sm_teid(mce_mbms_services_t * const mce_mbms_services, const sm_teid_t teid);

/** \brief Allocate a new MBMS Service context in the tree of known MBMS Services.
 * We restrict the MBMS Service struct to an MBMS Service of a single MBMS Service Area (single flow Id).
 * Uses the given MBMS Service Area ID and MBMS Service Area, which cannot change.
 * @returns the MBMS Service in case of success, NULL otherwise
 **/
struct mbms_service_s * mce_register_mbms_service(const tmgi_t * const tmgi, const mbms_service_area_id_t * const mbms_service_area_id, const teid_t mme_sm_teid);

/** \brief Remove an MBMS Service context of the tree of known MBMS Services.
 * \param tmgi_p 		  The TMGI of the MBMS Service
 * \param mbms_service_id The MBMS Service ID
 * \param teid_t		  The TEID of the MBMS Service
 **/
void mce_app_remove_mbms_service(
  struct tmgi_s * const tmgi_p, const mbms_service_area_id_t mbms_service_area_id, teid_t mme_teid_sm);

/** \brief Dump the MBMS Services present in the tree
 **/
void mce_app_dump_mbms_services(const mce_mbms_services_t * const mce_mbms_services);

/** \brief Update the MBMS Services with the given information.
 **/
void mce_app_update_mbms_service (const tmgi_t * const tmgi, const mbms_service_area_id_t old_mbms_service_area_id, const mbms_service_area_id_t new_mbms_service_area_id, const bearer_qos_t * const mbms_bearer_level_qos,
  const uint16_t mbms_flow_id, const mbms_ip_multicast_distribution_t * const mbms_ip_mc_dist, struct sockaddr * mbms_peer);

/** \brief Check if an MBMS Service with the given CTEID exists.
 * We don't use CTEID as a key.
 */
mbms_service_t                      *
mbms_cteid_in_list (const mce_mbms_services_t * const mce_mbms_services_p,
  const teid_t cteid);

/**
 * Get the MCE APP internal identifier for the MBMS Service context, uniquely based on the TMGI and per MBMS Service Area Id.
 */
mbms_service_index_t mce_get_mbms_service_index(const tmgi_t * tmgi, const mbms_service_area_id_t mbms_service_area_id);

/*
 * Update MBSFN Areas
 * todo: removal
 */
void mce_app_update_mbsfn_areas(const mbms_service_area_t * mbms_service_areas, const sctp_assoc_id_t assoc_id, const uint32_t m2_enb_id, mbsfn_areas_t * const mbsfn_areas);

/*
 * SM Procedures.
 */
mme_app_mbms_proc_t* mme_app_create_mbms_procedure(mbms_service_t * mbms_service, uint32_t delta_to_start_in_sec, uint32_t delta_to_start_in_usec, const mbms_session_duration_t * const mbms_session_duration);
void mme_app_delete_mbms_procedure(const mbms_service_t * mbms_service);

#endif /* FILE_MCE_APP_MBMS_SERVICE_CONTEXT_SEEN */

/* @} */