#include "camera.h"
#include <iostream>
#include <glm/ext.hpp>

Camera::Camera() {
  update_orientation();
}

const glm::dvec3& Camera::get_front() const {
  return front_;
}

const glm::dvec3& Camera::get_up() const {
  return up_;
};

const glm::dvec3& Camera::get_position() const {
  return position_;
}

void Camera::set_position(glm::dvec3 position) {
  position_ = position;
}

glm::dmat4 Camera::get_raw_view() const {
  return glm::lookAt(position_, position_ + glm::dvec3(front_), glm::dvec3(up_));
}

glm::mat4 Camera::get_view(glm::dvec3 camera_offset) const {
  glm::dvec3 adjusted_pos = position_ - camera_offset;
  return glm::lookAt(adjusted_pos, adjusted_pos + front_, up_);
}

void Camera::update_orientation() {
  front_.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
  front_.y = sin(glm::radians(pitch_));
  front_.z = -sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
  front_ = glm::normalize(front_);

  glm::dvec3 right = glm::normalize(glm::cross(glm::dvec3(0, 1, 0), front_));
  up_ = glm::normalize(glm::cross(front_, right));
}

void Camera::scale_translation_speed(double scale) {
  translation_speed_ = base_translation_speed_ * scale;
}
void Camera::scale_rotation_speed(double scale) {
  rotation_speed_ = base_rotation_speed_ * scale;
}

void Camera::set_base_translation_speed(double base_translation_speed) {
  base_translation_speed_ = base_translation_speed;
}

glm::vec3 Camera::get_position(glm::dvec3 world_offset) const {
  return position_ - world_offset;
}

void Camera::print() const {
  std::cout << "Camera at " << glm::to_string(position_) << std::endl;
  std::cout << "Yaw: " << yaw_ << ", Pitch: " << pitch_ << std::endl;
}

void Camera::set_orientation(double yaw, double pitch) {
  yaw_ = yaw;
  pitch_ = pitch;
  update_orientation();
}

double Camera::get_yaw() const { return yaw_; }
double Camera::get_pitch() const { return pitch_; }