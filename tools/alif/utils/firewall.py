""" firewall """
import json
import ctypes

# pylint: disable=too-few-public-methods
# pylint: disable=too-many-instance-attributes
# pylint: disable=invalid-name
# pylint: disable=attribute-defined-outside-init

INPUT_FILE = 'fw_cfg_cm.json'
OUTPUT_FILE = 'fw_cfg.bin'

c_uint32 = ctypes.c_uint32

class firewall_controller(ctypes.LittleEndianStructure):
    """ firewall_controller """
    _fields_ = [
                ("err",             c_uint32, 1),
                ("raz",             c_uint32, 1),
                ("sr_pwr",          c_uint32, 1),
                ("valid",           c_uint32, 1)
               ]

    def init_from_cfg(self, cfg):
        """ Initialize from JSON configuration """
        if cfg['valid']:
            self.valid = 1
            self.err = cfg['err']
            self.raz = cfg['raz']
            self.sr_pwr = cfg['sr_pwr']

class firewall_component(ctypes.LittleEndianStructure):
    """ firewall_component """
    _fields_ = [
                ("num_rgn_entries", c_uint32, 8),

                ("lock",            c_uint32, 2),
                ("err",             c_uint32, 1),
                ("raz",             c_uint32, 1),
                ("flt_cfg",         c_uint32, 2),
                ("fe_pwr",          c_uint32, 2),

                ("me_ctrl",         c_uint32, 3),
                ("int_msk",         c_uint32, 5),

                ("enable",          c_uint32, 1),
                ("valid",           c_uint32, 1),
               ]

    def init_from_cfg(self, cfg, num_rgns):
        """ Initialize from JSON configuration """
        self.num_rgn_entries = num_rgns
        self.lock = 0
        self.err = cfg['err']                 if 'err' in cfg     else 1
        self.raz = cfg['raz']                 if 'raz' in cfg     else 0
        self.flt_cfg = int(cfg['flt_cfg'], 0) if 'flt_cfg' in cfg else 0x2
        self.fe_pwr = cfg['fe_pwr']           if 'fe_pwr' in cfg  else 0
        self.me_ctrl = int(cfg['me_ctrl'], 0) if 'me_ctrl' in cfg else 0x0
        self.int_msk = int(cfg['int_msk'], 0) if 'int_msk' in cfg else 0x0
        self.enable = 1
        self.valid = 1

class fw_rgn_cmn(ctypes.LittleEndianStructure):
    """ fw_rgn_cmn """
    _fields_ = [
                ("rgn_indx",        c_uint32, 8),
                ("en",              c_uint32, 1),
                ("lock",            c_uint32, 1),
               ]

class fw_rgn_mpe(ctypes.LittleEndianStructure):
    """ fw_rgn_mpe """
    _fields_ = [
                ("mid",             c_uint32, 8),
                ("acc",             c_uint32, 12),
                ("any_mst",         c_uint32, 1),
                ("reserved",        c_uint32, 3),
                ("enable",          c_uint32, 1),
                ("valid",           c_uint32, 1),
               ]

    def init_from_cfg(self, cfg):
        """ Initialize from JSON configuration """
        self.valid = 1
        self.reserved = 0
        self.enable = 1
        self.mid = cfg['master_id']
        self.acc = int(cfg['permissions'], 0)
        self.any_mst = cfg['any_master']

class fw_rgn_pe2(ctypes.LittleEndianStructure):
    """ fw_rgn_pe2 """
    _fields_ = [
                ("size",            c_uint32, 7),
                ("mul_n_po2",       c_uint32, 1),
                ("addr_trans_en",   c_uint32, 1),
                ("ma_trans_en",     c_uint32, 1),
                ("reserved",        c_uint32, 6),
                ("trans_flags",     c_uint32, 16),
               ]

class firewall_region(ctypes.LittleEndianStructure):
    """ firewall_region """
    _fields_ = [
                ("cmn",             fw_rgn_cmn),
                ("mpe0",            fw_rgn_mpe),
                ("mpe1",            fw_rgn_mpe),
                ("mpe2",            fw_rgn_mpe),
                ("mpe3",            fw_rgn_mpe),
                ("base_addr",       c_uint32),
                ("out_uppr_addr",   c_uint32),
                ("pe2",             fw_rgn_pe2),
               ]

    def init_from_cfg(self, cfg, rgn_idx):
        """ Initialize from JSON configuration """
        self.cmn.rgn_indx = rgn_idx
        self.cmn.en = 1
        self.cmn.lock = 0

        self.mpe0.valid = 0
        self.mpe1.valid = 0
        self.mpe2.valid = 0
        self.mpe3.valid = 0
        for j_mpe in cfg['mpe_list']:
            if j_mpe['mpe_id'] == 0:
                self.mpe0.init_from_cfg(j_mpe)
            elif j_mpe['mpe_id'] == 1:
                self.mpe1.init_from_cfg(j_mpe)
            elif j_mpe['mpe_id'] == 2:
                self.mpe2.init_from_cfg(j_mpe)
            elif j_mpe['mpe_id'] == 3:
                self.mpe3.init_from_cfg(j_mpe)

        self.base_addr = int(cfg['start_address'], 0)

        self.out_uppr_addr = int(cfg['end_address'], 0)         if 'end_address' in cfg       else 0xFFFFFFFF
        self.pe2.size = int(cfg['size'], 0)                     if 'size' in cfg              else 0x0
        self.pe2.mul_n_po2 = cfg['mul_n_po2']                   if 'mul_n_po2' in cfg         else 0
        self.pe2.addr_trans_en = cfg['addr_trans_en']           if 'addr_trans_en' in cfg     else 0
        self.pe2.ma_trans_en = cfg['ma_trans_en']               if 'ma_trans_en' in cfg       else 0
        self.pe2.trans_flags = int(cfg['translation_flags'], 0) if 'translation_flags' in cfg else 0x4

class protected_area(ctypes.LittleEndianStructure):
    """ protected_area """
    _fields_ = [
                ("component_id",    c_uint32),
                ("base_addr",       c_uint32),
                ("upper_addr",      c_uint32)
               ]

    def init_from_cfg(self, cfg):
        """ Initialize from JSON configuration """
        self.component_id = cfg['component_id']
        self.base_addr = int(cfg['start_address'], 0)
        self.upper_addr = int(cfg['end_address'], 0)


FW_COMP_NUM = 15
FW_PROT_AREA_NUM = 16

def process_components(cfg):
    """ process_components """
    
    # order the FC sections by the component_id
    ordered_cfg = [None for i in range(FW_COMP_NUM)] 
    for j_comp in cfg:
        comp_id = int(j_comp['component_id'])
        if comp_id < 0 or comp_id >= FW_COMP_NUM:
            continue
        ordered_cfg[comp_id] = j_comp
        
    components = [firewall_component() for i in range(FW_COMP_NUM)]
    configured_regions = []

    for j_comp in ordered_cfg:
        if j_comp == None:
            continue
            
        rgn_count = 0
        for j_rgn in j_comp['configured_regions']:
            rgn_idx = int(j_rgn['region_index'])
            if rgn_idx < 0 or rgn_idx > 255:
                continue

            rgn = firewall_region()
            rgn.init_from_cfg(j_rgn, rgn_idx)

            rgn_count += 1
            configured_regions.append(rgn)

        comp_id = int(j_comp['component_id'])
        comp = components[comp_id]
        comp.init_from_cfg(j_comp, rgn_count)

    return components, configured_regions

def firewall_json_to_bin(jsn, is_icv):
    """ Convert a JSON file into binary """
    #with open(INPUT_FILE, "r") as json_file:
    #    jsn = json.load(json_file)

    # read the firewall components and configured regions
    components, configured_regions = process_components(jsn['firewall_components'])

    # read the firewall controller
    controller = firewall_controller()
    if 'firewall_controller' in jsn:
        controller.init_from_cfg(jsn['firewall_controller'])

    # read the protected areas
    protected_areas = []
    if is_icv and 'protected_areas' in jsn:
        protected_areas = [protected_area() for i in range(FW_PROT_AREA_NUM)]
        curr_area = 0
        for j_prot in jsn['protected_areas']:
            prot_area = protected_areas[curr_area]
            prot_area.init_from_cfg(j_prot)

            curr_area += 1
            if curr_area >= FW_PROT_AREA_NUM:
                break
        # end for loop over the protected areas

    with open(OUTPUT_FILE, 'wb') as bin_file:
        for comp in components:
            bin_file.write(comp)
        bin_file.write(controller)
        if is_icv:
            for prot_area in protected_areas:
                bin_file.write(prot_area)
        for rgn in configured_regions:
            bin_file.write(rgn)

def bin_to_struct():
    """ Convert a binary file into Python structures """
    components = [firewall_component() for i in range(FW_COMP_NUM)]
    configured_regions = []
    ctrl = firewall_controller()
    protected_areas = [protected_area() for i in range(FW_PROT_AREA_NUM)]

    with open(OUTPUT_FILE, 'rb') as bin_file:
        for comp in components:
            # read the components section
            bin_file.readinto(comp)
            # create region entries for each configured region in this component
            for _ in range(comp.num_rgn_entries):
                configured_regions.append(firewall_region())

        # read the controller section
        bin_file.readinto(ctrl)

        # read the protected areas section
        for prot_area in protected_areas:
            bin_file.readinto(prot_area)

        # read the configured regions section
        for rgn in configured_regions:
            bin_file.readinto(rgn)

def main():
    """ main """
    json_to_bin()
    return 0

if __name__ == "__main__":
    main()
