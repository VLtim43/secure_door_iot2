import 'package:flutter/material.dart';

enum AuthMethod { biometric, pin, totp }

extension AuthMethodDetails on AuthMethod {
  String get label => switch (this) {
    AuthMethod.biometric => 'Phone biometric',
    AuthMethod.pin => 'PIN',
    AuthMethod.totp => 'TOTP',
  };

  IconData get icon => switch (this) {
    AuthMethod.biometric => Icons.fingerprint,
    AuthMethod.pin => Icons.pin_outlined,
    AuthMethod.totp => Icons.timer_outlined,
  };
}
