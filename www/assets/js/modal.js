/**
 * file: modal.js
 * brief: This file contains the functions for the modal that is used to display images in a larger format.
 * 
 * author: Noah Nickles
 * date: 1/22/2025
 * details: COP4635 Sys & Net II - Project 1
 */

'use strict';

function openModal(img) {
  const modal = document.getElementById('picModal');
  const modalImg = document.getElementById('img');
  const captionText = document.getElementById('caption');

  modal.style.display = 'flex';
  modalImg.src = img.src;
  captionText.innerHTML = img.alt;
}

function closeModal() {
  const modal = document.getElementById('picModal');
  modal.style.display = 'none';
}