/***************************************************************************/
/*                                                                         */
/*  gxvmort5.c                                                             */
/*                                                                         */
/*    TrueTypeGX/AAT mort table validation                                 */
/*    body for type5 (Contextual Glyph Insertion) subtable.                */
/*                                                                         */
/*  Copyright 2005 by suzuki toshiya, Masatake YAMATO, Red Hat K.K.,       */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/* gxvalid is derived from both gxlayout module and otvalid module.        */
/* Development of gxlayout was support of Information-technology Promotion */
/* Agency(IPA), Japan.                                                     */
/***************************************************************************/

#include "gxvmort.h"

  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_gxvmort


/*
 * mort subtable type5 (Contextual Glyph Insertion)
 * has format of StateTable with insertion-glyph-list
 * without name. the offset is given by glyphOffset in
 * entryTable. there's no table location declaration
 * like xxxTable.
 */

  typedef struct  GXV_mort_subtable_type5_StateOptRec_
  {
    FT_UShort   classTable;
    FT_UShort   stateArray;
    FT_UShort   entryTable;
#define  GXV_MORT_SUBTABLE_TYPE5_HEADER_SIZE GXV_STATETABLE_HEADER_SIZE
    FT_UShort*  classTable_length_p;
    FT_UShort*  stateArray_length_p;
    FT_UShort*  entryTable_length_p;
  }  GXV_mort_subtable_type5_StateOptRec,
    *GXV_mort_subtable_type5_StateOptRecData;


  FT_LOCAL_DEF( void )
  gxv_mort_subtable_type5_subtable_setup( FT_UShort      table_size,
                                          FT_UShort      classTable,
                                          FT_UShort      stateArray,
                                          FT_UShort      entryTable,
                                          FT_UShort*     classTable_length_p,
                                          FT_UShort*     stateArray_length_p,
                                          FT_UShort*     entryTable_length_p,
                                          GXV_Validator  valid )
  {
    GXV_mort_subtable_type5_StateOptRecData  optdata = valid->statetable.optdata;
    gxv_StateTable_subtable_setup( table_size,
                                   classTable,
                                   stateArray,
                                   entryTable,
                                   classTable_length_p,
                                   stateArray_length_p,
                                   entryTable_length_p,
                                   valid );

    optdata->classTable = classTable;
    optdata->stateArray = stateArray;
    optdata->entryTable = entryTable;
    optdata->classTable_length_p = classTable_length_p;
    optdata->stateArray_length_p = stateArray_length_p;
    optdata->entryTable_length_p = entryTable_length_p;
  }



  static void
  gxv_mort_subtable_type5_InsertList_validate( FT_UShort      offset,
                                               FT_UShort      count,
                                               FT_Bytes       table,
                                               FT_Bytes       limit,
                                               GXV_Validator  valid )
  {
    /*
     * we don't know the range of insertion-glyph-list.
     * set range by whole of state table
     */
    FT_Bytes  p = table + offset;
    GXV_mort_subtable_type5_StateOptRecData  optdata = valid->statetable.optdata;

    if ( optdata->classTable < offset &&
         offset < optdata->classTable + *(optdata->classTable_length_p) )
      GXV_TRACE(( " offset runs into ClassTable" ));
    if ( optdata->stateArray < offset &&
         offset < optdata->stateArray + *(optdata->stateArray_length_p) )
      GXV_TRACE(( " offset runs into StateArray" ));
    if ( optdata->entryTable < offset &&
         offset < optdata->entryTable + *(optdata->entryTable_length_p) )
      GXV_TRACE(( " offset runs into EntryTable" ));

    while ( p < table + offset + ( count * 2 ) )
    {
      FT_UShort insert_glyphID;

      GXV_LIMIT_CHECK( 2 );
      insert_glyphID = FT_NEXT_USHORT( p );
      GXV_TRACE(( " 0x%04x", insert_glyphID ));
    }

    GXV_TRACE(( "\n" ));
  }


  static void
  gxv_mort_subtable_type5_entry_validate( FT_Byte                         state,
                                          FT_UShort                       flags,
                                          GXV_StateTable_GlyphOffsetDesc  glyphOffset,
                                          FT_Bytes                        table,
                                          FT_Bytes                        limit,
                                          GXV_Validator                   valid )
  {
    FT_Bool    setMark;
    FT_Bool    dontAdvance;
    FT_Bool    currentIsKashidaLike;
    FT_Bool    markedIsKashidaLike;
    FT_Bool    currentInsertBefore;
    FT_Bool    markedInsertBefore;
    FT_Byte    currentInsertCount;
    FT_Byte    markedInsertCount;
    FT_UShort  currentInsertList;
    FT_UShort  markedInsertList;


    setMark              = ( flags >> 15 ) & 1;
    dontAdvance          = ( flags >> 14 ) & 1;
    currentIsKashidaLike = ( flags >> 13 ) & 1;
    markedIsKashidaLike  = ( flags >> 12 ) & 1;
    currentInsertBefore  = ( flags >> 11 ) & 1;
    markedInsertBefore   = ( flags >> 10 ) & 1;
    currentInsertCount   = ( flags & 0x03E0 ) / 0x0020;
    markedInsertCount    = ( flags & 0x001F );
    currentInsertList    = glyphOffset.ul / 0x00010000;
    markedInsertList     = glyphOffset.ul & 0x0000FFFF;

    if ( 0 != currentInsertList && 0 != currentInsertCount )
    {
      gxv_mort_subtable_type5_InsertList_validate( currentInsertList,
                                                   currentInsertCount,
                                                   table,
                                                   limit,
                                                   valid );
    }

    if ( 0 != markedInsertList && 0 != markedInsertCount )
    {
      gxv_mort_subtable_type5_InsertList_validate( markedInsertList,
                                                   markedInsertCount,
                                                   table,
                                                   limit,
                                                   valid );
    }
  }


  static void
  gxv_mort_subtable_type5_validate( FT_Bytes       table,
                                    FT_Bytes       limit,
                                    GXV_Validator  valid )
  {
    FT_Bytes  p = table;
    GXV_mort_subtable_type5_StateOptRec      et_rec;
    GXV_mort_subtable_type5_StateOptRecData  et = &et_rec;


    GXV_NAME_ENTER( "mort chain subtable type5 (Glyph Insertion)" );

    GXV_LIMIT_CHECK( GXV_MORT_SUBTABLE_TYPE5_HEADER_SIZE );

    valid->statetable.optdata               = et;
    valid->statetable.optdata_load_func     = NULL;
    valid->statetable.subtable_setup_func   = gxv_mort_subtable_type5_subtable_setup;
    valid->statetable.entry_glyphoffset_fmt = GXV_GLYPHOFFSET_ULONG;
    valid->statetable.entry_validate_func   = gxv_mort_subtable_type5_entry_validate;
    gxv_StateTable_validate( p, limit, valid );
    GXV_EXIT;
  }


/* END */