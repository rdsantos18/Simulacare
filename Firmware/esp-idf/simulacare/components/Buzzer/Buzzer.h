 /* Copyright (c) 2017 pcbreflux. All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>. *
 */

#ifndef MAIN_BUZZER_H_
#define MAIN_BUZZER_H_

void buzzer_task(void *pvParameters);
void play_on(void);
void play_off(void);
void play_bat_1(void);
void play_bat_2(void);

#endif /* MAIN_BUZZER_H_ */

