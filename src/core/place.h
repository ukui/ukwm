/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/* Ukwm window placement */

/*
 * Copyright (C) 2001 Havoc Pennington
 * Copyright (C) 2017 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef META_PLACE_H
#define META_PLACE_H

#include "window-private.h"
#include "frame.h"

void meta_window_process_placement (MetaWindow        *window,
                                    MetaPlacementRule *placement_rule,
                                    int               *x,
                                    int               *y);

void meta_window_place (MetaWindow *window,
                        int         x,
                        int         y,
                        int        *new_x,
                        int        *new_y);

#endif
