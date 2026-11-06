import 'package:flutter/material.dart';

enum AuthMethod { rfid, biometric, pin, totp }

extension AuthMethodDetails on AuthMethod {
  String get label => switch (this) {
    AuthMethod.rfid => 'RFID',
    AuthMethod.biometric => 'Phone biometric',
    AuthMethod.pin => 'PIN',
    AuthMethod.totp => 'TOTP',
  };

  IconData get icon => switch (this) {
    AuthMethod.rfid => Icons.badge_outlined,
    AuthMethod.biometric => Icons.fingerprint,
    AuthMethod.pin => Icons.pin_outlined,
    AuthMethod.totp => Icons.timer_outlined,
  };
}
