#!/usr/bin/perl -s
# SPDX-License-Identifier: BSD-3-Clause-Clear
# Copyright (c) 2022, MediaTek Inc.

$output = $ARGV[0]; #param1
$labels = $ARGV[1]; #param2

@product = split(' ', $labels);#Array of labels
for($i = 0; $i <= $#product; $i++){
	print $product[$i]."\t";
}
print "\n";

open(OUTPUT, ">$output") || die "$0 : can't open $output for writing\n";
($_ = $0) =~ s%.*/%%; #% is separator instead of "/", because "/" show in matched string.
print OUTPUT "/* SPDX-License-Identifier: BSD-3-Clause-Clear\n *\n * Copyright (c) 2022, MediaTek Inc.*/\n";
print OUTPUT "/* DO NOT EDIT - the file is created automatically by ".$_."*/\n";
if($output =~ m%cldma%){
	for($i = 0; $i <= $#product; $i++){
		@suffix = split('_', $product[$i]);
		if($#suffix == 1){
			print OUTPUT "extern struct cldma_hw_ops hw_ops_name(".$suffix[0].");\n";
		} else {
			break;
		}
	}
	print OUTPUT "\n";
	print OUTPUT "static struct cldma_hw_ops_desc cldma_hw_ops_tbl[] = {\n";
	for($i = 0; $i <= $#product; $i++){
		@suffix = split('_', $product[$i]);
		if($#suffix == 1){
			print OUTPUT "\t{".hex($suffix[1]).", &hw_ops_name(".$suffix[0].")},\n" ;
		} else {
			break;
		}
	}
	print OUTPUT "\t{0, NULL},\n";
	print OUTPUT "};\n";
}elsif($output =~ m%dpmaif%){
	for($i = 0; $i <= $#product; $i++){
		@suffix = split('_', $product[$i]);
		if($#suffix == 1){
			print OUTPUT "extern struct dpmaif_drv_ops drv_ops_name(".$suffix[0].");\n";
		} else {
			break;
		}
	}
	print OUTPUT "\n";
	print OUTPUT "static struct dpmaif_drv_ops_desc dpmaif_drv_ops_tbl[] = {\n";
	for($i = 0; $i <= $#product; $i++){
		@suffix = split('_', $product[$i]);
		if($#suffix == 1){
			print OUTPUT "\t{".hex($suffix[1]).", &drv_ops_name(".$suffix[0].")},\n" ;
		} else {
			break;
		}
	}
	print OUTPUT "\t{0, NULL},\n";
	print OUTPUT "};\n";
}else{
	for($i = 0; $i <= $#product; $i++){
		@suffix = split('_', $product[$i]);
		@sub_suffix = split('x', $suffix[1]);
		if($#suffix == 1){
			print OUTPUT "extern const struct mtk_pci_dev_cfg mtk_dev_cfg_".$sub_suffix[1].";\n";
			if(hex($suffix[1]) == 0x4d75){
				print OUTPUT"extern const struct mtk_pci_dev_cfg mtk_dev_cfg_4d80;\n" ;
			}
		} else {
			break;
		}
	}
	print OUTPUT "\n";
	print OUTPUT "static const struct  pci_device_id mtk_pci_ids[] = {\n";
	for($i = 0; $i <= $#product; $i++){
		@suffix = split('_', $product[$i]);
		@sub_suffix = split('x', $suffix[1]);
		if($#suffix == 1){
			print OUTPUT"\tMTK_PCI_DEV_CFG(".$suffix[1].", mtk_dev_cfg_".$sub_suffix[1]."),\n" ;
			if(hex($suffix[1]) == 0x4d75){
				print OUTPUT"\tMTK_PCI_DEV_CFG(0x4d80, mtk_dev_cfg_4d80),\n" ;
			}
		} else {
			break;
		}
	}
	print OUTPUT "\t{/* end: all zeroes */}\n";
	print OUTPUT "};\n";
}
close(OUTPUT);
