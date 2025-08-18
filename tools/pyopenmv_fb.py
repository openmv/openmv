#!/usr/bin/env python
# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2025 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2025 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# An example script using pyopenmv to grab the framebuffer.

import sys
import numpy as np
import pygame
import pyopenmv
import argparse
import time

test_script = """
import sensor, image, time
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 2000)
clock = time.clock()

while(True):
    clock.tick()
    img = sensor.snapshot()
    print(clock.fps(), " FPS")
"""

bench_script = """
import sensor, image, time
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.VGA)
img = sensor.snapshot().compress()
while(True):
    img.flush()
"""

def addr_to_symbol(symbols, address):
    # Binary search for speed
    lo, hi = 0, len(symbols) - 1
    while lo <= hi:
        mid = (lo + hi) // 2
        start, end, name = symbols[mid]
        if start <= address < end:
            return name
        elif address < start:
            hi = mid - 1
        else:
            lo = mid + 1
    return None

def get_color_by_percentage(percentage, base_color=(220, 220, 220)):
    """
    Return a color based on percentage with fine-grained intensity levels.
    """
    def clamp(value):
        return max(0, min(255, int(value)))
    
    if percentage >= 50:
        # Very high - bright red
        intensity = min(1.0, (percentage - 50) / 50)
        return (255, clamp(120 - 120 * intensity), clamp(120 - 120 * intensity))
    elif percentage >= 30:
        # High - red-orange
        intensity = (percentage - 30) / 20
        return (255, clamp(160 + 40 * intensity), clamp(160 - 40 * intensity))
    elif percentage >= 20:
        # Medium-high - orange
        intensity = (percentage - 20) / 10
        return (255, clamp(200 + 55 * intensity), clamp(180 - 20 * intensity))
    elif percentage >= 15:
        # Medium - yellow-orange
        intensity = (percentage - 15) / 5
        return (255, clamp(220 + 35 * intensity), clamp(180 + 20 * intensity))
    elif percentage >= 10:
        # Medium-low - yellow
        intensity = (percentage - 10) / 5
        return (clamp(255 - 75 * intensity), 255, clamp(180 + 75 * intensity))
    elif percentage >= 5:
        # Low - light green
        intensity = (percentage - 5) / 5
        return (clamp(180 + 75 * intensity), 255, clamp(180 + 75 * intensity))
    elif percentage >= 2:
        # Very low - green
        intensity = (percentage - 2) / 3
        return (clamp(160 + 95 * intensity), clamp(255 - 55 * intensity), clamp(160 + 95 * intensity))
    elif percentage >= 1:
        # Minimal - light blue-green
        intensity = (percentage - 1) / 1
        return (clamp(140 + 120 * intensity), clamp(200 + 55 * intensity), clamp(255 - 95 * intensity))
    else:
        # Zero or negligible - base color
        return base_color

def draw_rounded_rect(surface, color, rect, radius=5):
    x, y, w, h = rect
    if w <= 0 or h <= 0:
        return
    pygame.draw.rect(surface, color, (x + radius, y, w - 2*radius, h))
    pygame.draw.rect(surface, color, (x, y + radius, w, h - 2*radius))
    pygame.draw.circle(surface, color, (x + radius, y + radius), radius)
    pygame.draw.circle(surface, color, (x + w - radius, y + radius), radius)
    pygame.draw.circle(surface, color, (x + radius, y + h - radius), radius)
    pygame.draw.circle(surface, color, (x + w - radius, y + h - radius), radius)


def draw_table(overlay_surface, config, title, headers, col_widths):
    """Draw the common table background, title, and header."""
    # Draw main table background
    table_rect = (0, 0, config['width'], config['height'])
    draw_rounded_rect(overlay_surface, config['colors']['bg'], table_rect, int(8 * config['scale_factor']))
    pygame.draw.rect(overlay_surface, config['colors']['border'], table_rect, max(1, int(2 * config['scale_factor'])))
    
    # Table title
    title_text = config['fonts']['title'].render(title, True, config['colors']['header_text'])
    title_rect = title_text.get_rect()
    title_x = (config['width'] - title_rect.width) // 2
    overlay_surface.blit(title_text, (title_x, int(12 * config['scale_factor'])))
    
    # Header
    header_y = int(50 * config['scale_factor'])
    header_height = int(40 * config['scale_factor'])
    
    # Draw header background
    header_rect = (int(5 * config['scale_factor']), header_y, 
                   config['width'] - int(10 * config['scale_factor']), header_height)
    draw_rounded_rect(overlay_surface, config['colors']['header_bg'], header_rect, int(4 * config['scale_factor']))
    
    # Draw header text and separators
    current_x = int(10 * config['scale_factor'])
    for i, (header, width) in enumerate(zip(headers, col_widths)):
        header_surface = config['fonts']['header'].render(header, True, config['colors']['header_text'])
        overlay_surface.blit(header_surface, (current_x, header_y + int(6 * config['scale_factor'])))
        
        if i < len(headers) - 1:
            sep_x = current_x + width - int(5 * config['scale_factor'])
            pygame.draw.line(overlay_surface, config['colors']['border'],
                           (sep_x, header_y + 2), (sep_x, header_y + header_height - 2), 1)
        current_x += width


def draw_event_table(overlay_surface, config, profile_data, profile_mode, symbols):
    """Draw the event counter mode table."""

    # Prepare data
    num_events = len(profile_data[0]['events']) if profile_data else 0
    if not num_events:
        sorted_data = sorted(profile_data, key=lambda x: x['address'])
    else:
        sort_func = lambda x: x['events'][0] // max(1, x['call_count'])
        sorted_data = sorted(profile_data, key=sort_func, reverse=True)
    
    headers = ["Function"] + [f"E{i}" for i in range(num_events)]
    proportions = [0.30] + [0.70/num_events] * num_events
    col_widths = [config['width'] * prop for prop in proportions]
    profile_mode = "Exclusive" if profile_mode else "Inclusive"
    
    # Calculate event totals for percentage calculation
    event_totals = [0] * num_events
    for record in sorted_data:
        for i, event_count in enumerate(record['events']):
            event_totals[i] += event_count // max(1, record['call_count'])
    
    # Draw table structure
    draw_table(overlay_surface, config, f"Event Counters ({profile_mode})", headers, col_widths)
    
    # Draw data rows
    row_height = int(30 * config['scale_factor'])
    data_start_y = int(50 * config['scale_factor'] + 40 * config['scale_factor'] + 8 * config['scale_factor'])
    available_height = config['height'] - data_start_y - int(60 * config['scale_factor'])
    visible_rows = min(len(sorted_data), available_height // row_height)
    
    for i in range(visible_rows):
        record = sorted_data[i]
        row_y = data_start_y + i * row_height
        
        # Draw row background
        row_color = config['colors']['row_alt'] if i % 2 == 0 else config['colors']['row_normal']
        row_rect = (int(5 * config['scale_factor']), row_y, 
                   config['width'] - int(10 * config['scale_factor']), row_height)
        pygame.draw.rect(overlay_surface, row_color, row_rect)
        
        # Function name
        name = addr_to_symbol(symbols, record['address']) if symbols else "<no symbols>"
        max_name_chars = int(col_widths[0] // (11 * config['scale_factor']))
        display_name = name if len(name) <= max_name_chars else name[:max_name_chars - 3] + "..."
        
        row_data = [display_name]
        
        # Event data
        for j, event_count in enumerate(record['events']):
            event_scale = ""
            event_count //= max(1, record['call_count'])
            if event_count > 1_000_000_000:
                event_count //= 1_000_000_000
                event_scale = "B"
            elif event_count > 1_000_000:
                event_count //= 1_000_000
                event_scale = "M"
            row_data.append(f"{event_count:,}{event_scale}")
        
        # Determine row color based on sorting key (event 0)
        if len(record['events']) > 0 and event_totals[0] > 0:
            sort_key_value = record['events'][0] // max(1, record['call_count'])
            percentage = (sort_key_value / event_totals[0] * 100)
            row_text_color = get_color_by_percentage(percentage, config['colors']['content_text'])
        else:
            row_text_color = config['colors']['content_text']
        
        # Draw row data with uniform color
        current_x = 10
        for j, (data, width) in enumerate(zip(row_data, col_widths)):
            text_surface = config['fonts']['content'].render(str(data), True, row_text_color)
            overlay_surface.blit(text_surface, (current_x, row_y + int(8 * config['scale_factor'])))
            
            if j < len(row_data) - 1:
                sep_x = current_x + width - 8
                pygame.draw.line(overlay_surface, (60, 70, 85),
                               (sep_x, row_y), (sep_x, row_y + row_height), 1)
            current_x += width
    
    # Draw summary
    summary_y = config['height'] - int(50 * config['scale_factor'])
    total_functions = len(profile_data)
    grand_total = sum(event_totals)
    summary_text = (
        f"Profiles: {total_functions} | "
        f"Events: {num_events} | "
        f"Total Events: {grand_total:,}"
    )
    
    summary_surface = config['fonts']['summary'].render(summary_text, True, config['colors']['content_text'])
    summary_rect = summary_surface.get_rect()
    summary_x = (config['width'] - summary_rect.width) // 2
    overlay_surface.blit(summary_surface, (summary_x, summary_y))
    
    # Instructions
    instruction_str = "Press 'P' to toggle event counter overlay"
    instruction_text = config['fonts']['instruction'].render(instruction_str, True, (180, 180, 180))
    overlay_surface.blit(instruction_text, (0, summary_y + int(20 * config['scale_factor'])))


def draw_profile_table(overlay_surface, config, profile_data, profile_mode, symbols):
    """Draw the profile mode table."""

    # Prepare data
    sort_func = lambda x: x['total_ticks']
    sorted_data = sorted(profile_data, key=sort_func, reverse=True)
    total_ticks_all = sum(record['total_ticks'] for record in profile_data)
    profile_mode = "Exclusive" if profile_mode else "Inclusive"
    
    headers = ["Function", "Calls", "Min", "Max", "Total", "Avg", "Cycles", "%"]
    proportions = [0.30, 0.08, 0.10, 0.10, 0.13, 0.10, 0.13, 0.05]
    col_widths = [config['width'] * prop for prop in proportions]
    
    # Draw table structure
    draw_table(overlay_surface, config, f"Performance Profile ({profile_mode})", headers, col_widths)
    
    # Draw data rows
    row_height = int(30 * config['scale_factor'])
    data_start_y = int(50 * config['scale_factor'] + 40 * config['scale_factor'] + 8 * config['scale_factor'])
    available_height = config['height'] - data_start_y - int(60 * config['scale_factor'])
    visible_rows = min(len(sorted_data), available_height // row_height)
    
    for i in range(visible_rows):
        record = sorted_data[i]
        row_y = data_start_y + i * row_height
        
        # Draw row background
        row_color = config['colors']['row_alt'] if i % 2 == 0 else config['colors']['row_normal']
        row_rect = (int(5 * config['scale_factor']), row_y, 
                   config['width'] - int(10 * config['scale_factor']), row_height)
        pygame.draw.rect(overlay_surface, row_color, row_rect)
        
        # Function name
        name = addr_to_symbol(symbols, record['address']) if symbols else "<no symbols>"
        max_name_chars = int(col_widths[0] // (11 * config['scale_factor']))
        display_name = name if len(name) <= max_name_chars else name[:max_name_chars - 3] + "..."
        
        # Calculate values
        call_count = record['call_count']
        min_ticks = record['min_ticks'] if call_count else 0
        max_ticks = record['max_ticks'] if call_count else 0
        total_ticks = record['total_ticks']
        avg_cycles = record['total_cycles'] // max(1, call_count)
        avg_ticks = total_ticks // max(1, call_count)
        percentage = (total_ticks / total_ticks_all * 100)
        
        ticks_scale = ""
        if total_ticks > 1_000_000_000:
            total_ticks //= 1_000_000
            ticks_scale = "M"
        
        row_data = [
            display_name,
            f"{call_count:,}",
            f"{min_ticks:,}",
            f"{max_ticks:,}",
            f"{total_ticks:,}{ticks_scale}",
            f"{avg_ticks:,}",
            f"{avg_cycles:,}",
            f"{percentage:.1f}%"
        ]
        
        # Determine row color based on percentage
        text_color = get_color_by_percentage(percentage, config['colors']['content_text'])
        
        # Draw row data
        current_x = int(10 * config['scale_factor'])
        for j, (data, width) in enumerate(zip(row_data, col_widths)):
            text_surface = config['fonts']['content'].render(str(data), True, text_color)
            overlay_surface.blit(text_surface, (current_x, row_y + int(8 * config['scale_factor'])))
            
            if j < len(row_data) - 1:
                sep_x = current_x + width - int(8 * config['scale_factor'])
                pygame.draw.line(overlay_surface, (60, 70, 85),
                               (sep_x, row_y), (sep_x, row_y + row_height), 1)
            current_x += width
    
    # Draw summary
    summary_y = config['height'] - int(50 * config['scale_factor'])
    total_calls = sum(record['call_count'] for record in profile_data)
    total_cycles = sum(record['total_cycles'] for record in profile_data)
    total_ticks_summary = sum(record['total_ticks'] for record in profile_data)
    
    summary_text = (
        f"Profiles: {len(profile_data)} | "
        f"Total Calls: {total_calls:,} | "
        f"Total Ticks: {total_ticks_summary:,} | "
        f"Total Cycles: {total_cycles:,}"
    )
    
    summary_surface = config['fonts']['summary'].render(summary_text, True, config['colors']['content_text'])
    summary_rect = summary_surface.get_rect()
    summary_x = (config['width'] - summary_rect.width) // 2
    overlay_surface.blit(summary_surface, (summary_x, summary_y))
    
    # Instructions
    instruction_str = "Press 'P' to toggle event counter overlay"
    instruction_text = config['fonts']['instruction'].render(instruction_str, True, (180, 180, 180))
    overlay_surface.blit(instruction_text, (0, summary_y + int(20 * config['scale_factor'])))

def draw_profile_overlay(screen, screen_width, screen_height, profile_data,
                         profile_mode, profile_type, scale, symbols, alpha=250):
    """Main entry point for drawing the profile overlay."""
    # Calculate dimensions and create surface
    base_width, base_height = 800, 800
    screen_width *= scale
    screen_height *= scale
    scale_factor = min(screen_width / base_width, screen_height / base_height)
    
    overlay_surface = pygame.Surface((screen_width, screen_height), pygame.SRCALPHA)
    overlay_surface.set_alpha(alpha)
    
    # Setup common configuration
    config = {
        'width': screen_width,
        'height': screen_height,
        'scale_factor': scale_factor,
        'colors': {
            'bg': (40, 50, 65),
            'border': (70, 80, 100),
            'header_bg': (60, 80, 120),
            'header_text': (255, 255, 255),
            'content_text': (220, 220, 220),
            'row_alt': (35, 45, 60),
            'row_normal': (45, 55, 70)
        },
        'fonts': {
            'title': pygame.font.SysFont("arial", int(28 * scale_factor), bold=True),
            'header': pygame.font.SysFont("monospace", int(20 * scale_factor), bold=True),
            'content': pygame.font.SysFont("monospace", int(18 * scale_factor)),
            'summary': pygame.font.SysFont("arial", int(20 * scale_factor)),
            'instruction': pygame.font.SysFont("arial", int(22 * scale_factor))
        }
    }

    # Draw based on mode
    if profile_type == 1:
        draw_profile_table(overlay_surface, config, profile_data, profile_mode, symbols)
    elif profile_type == 2:
        draw_event_table(overlay_surface, config, profile_data, profile_mode, symbols)
    
    screen.blit(overlay_surface, (0, 0))

def pygame_test(port, script, poll_rate, scale, benchmark, symbols):
    # init pygame
    pygame.init()
    pyopenmv.disconnect()

    connected = False
    for i in range(10):
        try:
            # opens CDC port.
            # Set small timeout when connecting
            pyopenmv.init(port, baudrate=921600, timeout=0.050)
            connected = True
            break
        except Exception as e:
            connected = False
            time.sleep(0.100)
    
    if not connected:
        print("Failed to connect to OpenMV's serial port.\n"
              "Please install OpenMV's udev rules first:\n"
              "sudo cp openmv/udev/50-openmv.rules /etc/udev/rules.d/\n"
              "sudo udevadm control --reload-rules\n\n")
        sys.exit(1)
    
    # Set higher timeout after connecting for lengthy transfers.
    pyopenmv.set_timeout(1*2) # SD Cards can cause big hicups.
    pyopenmv.stop_script()
    pyopenmv.enable_fb(True)
    pyopenmv.reset_profiler()

    # Configure some event counters.
    pyopenmv.set_event_counter(0, 0x0039)
    pyopenmv.set_event_counter(1, 0x0023)
    pyopenmv.set_event_counter(2, 0x0024)
    pyopenmv.set_event_counter(3, 0x0001)
    pyopenmv.set_event_counter(4, 0x0003)
    pyopenmv.set_event_counter(5, 0xC102)
    pyopenmv.set_event_counter(6, 0x02CC)
    pyopenmv.set_event_counter(7, 0xC303)

    pyopenmv.exec_script(script)
    
    # init screen
    running = True
    screen = None

    # Profiling control
    profile_type = 0
    profile_mode = 0
    profile_data = []
    last_profile_read = 0
    
    clock = pygame.time.Clock()
    fps_clock = pygame.time.Clock()
    font = pygame.font.SysFont("monospace", 30)

    if not benchmark:
        pygame.display.set_caption("OpenMV Camera")
    else:
        pygame.display.set_caption("OpenMV Camera (Benchmark)")
        screen = pygame.display.set_mode((640, 120), pygame.DOUBLEBUF, 32)

    try:
        while running:
            # Read state
            w, h, data, size, text, fmt, profiling = pyopenmv.read_state()
    
            if text is not None:
                print(text, end="")
    
            # Read profiling data (maximum 10Hz)
            if profiling and profile_type:
                current_time = time.time()
                if current_time - last_profile_read >= 0.1:  # 10Hz = 0.1s interval
                    tmp_data = pyopenmv.read_profile()
                    if tmp_data:
                        profile_data = tmp_data
                    last_profile_read = current_time

            #if profile_data:
            #    for r in profile_data:
            #        print(f"Func: {addr_to_symbol(symbols, r['address'])}@0x{r['address']:x} ")
            #        print(f"Call: {addr_to_symbol(symbols, r['caller'])}@0x{r['caller']:x}")
            #    sys.exit(0)

            if data is not None:
                fps = fps_clock.get_fps()
    
                # Create image from RGB888
                if not benchmark:
                    image = pygame.image.frombuffer(data.flat[0:], (w, h), 'RGB')
                    image = pygame.transform.smoothscale(image, (w * scale, h * scale))
    
                if screen is None:
                    screen = pygame.display.set_mode((w * scale, h * scale), pygame.DOUBLEBUF, 32)
    
                # blit stuff
                if benchmark:
                    screen.fill((0, 0, 0))
                else:
                    screen.blit(image, (0, 0))
                
                # FPS text
                fps_text = f"{fps:.2f} FPS {fps * size / 1024**2:.2f} MB/s {w}x{h} {fmt}"
                screen.blit(font.render(fps_text, 5, (255, 0, 0)), (0, 0))
                
                # Draw profile overlay if enabled
                if profile_type and profile_data:
                    draw_profile_overlay(screen, w, h, profile_data, profile_mode, profile_type, scale, symbols)
    
                # update display
                pygame.display.flip()
                fps_clock.tick(1000//poll_rate)
    
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                     running = False
                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_ESCAPE:
                        running = False
                    elif event.key == pygame.K_c:
                        pygame.image.save(image, "capture.png")
                    elif event.key == pygame.K_p:
                        profile_type = (profile_type + 1) % 3
                    elif event.key == pygame.K_m:
                        profile_mode = not profile_mode
                        pyopenmv.set_profile_mode(profile_mode)
                    elif event.key == pygame.K_r:
                        pyopenmv.reset_profiler()

            clock.tick(1000//poll_rate)
            
    except KeyboardInterrupt:
        pass
    
    pygame.quit()
    pyopenmv.stop_script()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='pyopenmv module')
    parser.add_argument('--port', action = 'store', help='Serial port (dev/ttyACM0)', default='/dev/ttyACM0')
    parser.add_argument("--script", action = "store", default=None, help = "Script file")
    parser.add_argument('--poll', action = 'store', help='Poll rate (default 4ms)', default=4, type=int)
    parser.add_argument('--bench', action = 'store_true', help='Run throughput benchmark.', default=False)
    parser.add_argument('--scale', action = 'store', help='Set frame scaling factor (default 4x).', default=4, type=int)
    parser.add_argument('--firmware', action = 'store', help='Firmware for address to symbol', default=None)

    args = parser.parse_args()
    if args.script is not None:
        with open(args.script) as f:
            args.script = f.read()
    else:
        args.script = bench_script if args.bench else test_script

    symbols = []

    if args.firmware:
        from elftools.elf.elffile import ELFFile
    
        with open(args.firmware, 'rb') as f:
            elf = ELFFile(f)
            symtab = elf.get_section_by_name('.symtab')
            if not symtab:
                raise ValueError("No symbol table found in ELF.")
    
            for sym in symtab.iter_symbols():
                addr = sym['st_value']
                size = sym['st_size']
                name = sym.name
                if name and size > 0:  # ignore empty symbols
                    symbols.append((addr, addr + size, name))

        symbols.sort()

    pygame_test(args.port, args.script, args.poll, args.scale, args.bench, symbols)
