import os
import cv2
from multiprocessing import Pool
import numpy as np
from tqdm.auto import tqdm

import argparse

"""
Usecase example: 

python create_dataset.py --input_dir /path/to/input_images --hr_dir /path/to/save_hr_patches --blr_dir /path/to/save_blurred_patches --kernel_path blur_kernel.csv --stride 0.75 --patch_size 256 --nb_img -1

"""

def patch_image(img_path, hr_out_folder, blr_out_folder, patch_size, blur_kernel, stride_ratio):
    
    """
    Extracts patches from an input image, applies a blur, and saves the patches to separate directories.

    Args:
        img_path (str): Path to the input image.
        hr_out_folder (str): Path to the directory to save high-resolution patches.
        blr_out_folder (str): Path to the directory to save blurred patches.
        patch_size (int): Size of the patches to extract.
        blur_kernel (numpy.ndarray): Kernel for blurring the image.
        stride_ratio (float): Ratio of stride to patch size.

    Returns:
        None
    """

    img = cv2.imread(img_path)

    height, width, _ = img.shape
    
    stride = int(stride_ratio * patch_size)
    
    blur_img = cv2.filter2D(img, -1, blur_kernel)
    
    for i in range(0, height-patch_size, stride):
        
        for j in range(0, width-patch_size, stride):
            
            hr_patch = img[i:i+patch_size, j:j+patch_size]
            blr_patch = blur_img[i:i+patch_size, j:j+patch_size][::2,::2]
            
            img_name = os.path.basename(img_path)
            
            hr_patch_path = os.path.join(hr_out_folder, f"gt_{img_name[:-4]}_{i}_{j}.png")
            blr_patch_path = os.path.join(blr_out_folder,
                                           f"blr_{img_name[:-4]}_{i}_{j}.png")

            cv2.imwrite(hr_patch_path, hr_patch)
            cv2.imwrite(blr_patch_path, blr_patch)

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('--input_dir', default=None, type=str, help='Path to input image directory')
    parser.add_argument('--hr_dir', default='dz_dataset/train/hr/', type=str, help='Path to save high-resolution patches')
    parser.add_argument('--blr_dir', default='dz_dataset/train/blr/', type=str, help='Path to save blurred low res patches')
    parser.add_argument('--kernel_path', default='PSF_E10x2.csv', type=str, help='Path to blur kernel')
    parser.add_argument('--stride', default=0.75, type=float, help='Stride ratio for patch extraction')
    parser.add_argument('--patch_size', default=256, type=int, help='Size of the patches to extract')
    parser.add_argument('--nb_img', default=-1, type=int, help='Number of images to process, -1 for full dataset')
    
    args = parser.parse_args()

    input_images_folder = args.input_dir
    hr_output_images_folder = args.hr_dir
    blr_output_images_folder = args.blr_dir
    kernel_path = args.kernel_path
    stride_ratio = args.stride
    patch_size = args.patch_size
    nb_img = args.nb_img
    
    blur_kernel = np.genfromtxt(kernel_path, delimiter=';')

    if hr_output_images_folder == None:
        hr_output_images_folder = "hr_patch"
    
    if blr_output_images_folder == None:
        blr_output_images_folder = "blr_patch"
    
    os.makedirs(hr_output_images_folder, exist_ok=True)
    os.makedirs(blr_output_images_folder, exist_ok=True)
    input_images = [f for f in os.listdir(input_images_folder) if f.endswith(('.png'))]
    
    if nb_img == -1:
        nb_img = len(input_images)       

    with Pool() as p:
        p.starmap(patch_image, [(os.path.join(input_images_folder, img_name), 
                                 hr_output_images_folder,
                                 blr_output_images_folder, patch_size, blur_kernel, stride_ratio) for img_name in input_images[:nb_img]])