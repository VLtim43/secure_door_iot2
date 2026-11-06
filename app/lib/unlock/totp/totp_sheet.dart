import 'dart:async';
import 'dart:math';

import 'package:flutter/material.dart';

class TotpSheet extends StatefulWidget {
  const TotpSheet({super.key, required this.approved, required this.onApprove});

  final bool approved;
  final VoidCallback onApprove;

  @override
  State<TotpSheet> createState() => _TotpSheetState();
}

class _TotpSheetState extends State<TotpSheet> {
  static const _intervalMs = 30000;
  late final Timer _timer;
  late int _period;
  late String _code;
  int _remainingMs = _intervalMs;

  @override
  void initState() {
    super.initState();
    final now = DateTime.now().millisecondsSinceEpoch;
    _period = now ~/ _intervalMs;
    _code = _demoCode(_period);
    _remainingMs = _intervalMs - now % _intervalMs;
    _timer = Timer.periodic(
      const Duration(milliseconds: 100),
      (_) => _updateCode(),
    );
  }

  void _updateCode() {
    final now = DateTime.now().millisecondsSinceEpoch;
    final period = now ~/ _intervalMs;
    final remaining = _intervalMs - now % _intervalMs;
    setState(() {
      if (_period != period) {
        _period = period;
        _code = _demoCode(period);
      }
      _remainingMs = remaining;
    });
  }

  String _demoCode(int period) =>
      Random(period).nextInt(1000000).toString().padLeft(6, '0');

  @override
  void dispose() {
    _timer.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final colors = Theme.of(context).colorScheme;
    return SafeArea(
      child: Padding(
        padding: const EdgeInsets.fromLTRB(24, 8, 24, 24),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            Text('TOTP', style: Theme.of(context).textTheme.headlineSmall),
            const SizedBox(height: 8),
            const Text('Demo code only · Not synced with a backend.'),
            const SizedBox(height: 24),
            Center(
              child: SizedBox(
                width: 210,
                height: 210,
                child: Stack(
                  alignment: Alignment.center,
                  children: [
                    SizedBox.expand(
                      child: CircularProgressIndicator(
                        value: _remainingMs / _intervalMs,
                        strokeWidth: 10,
                        backgroundColor: colors.primaryContainer,
                        color: colors.primary,
                      ),
                    ),
                    Column(
                      mainAxisSize: MainAxisSize.min,
                      children: [
                        Text(
                          _code,
                          style: Theme.of(context).textTheme.headlineLarge
                              ?.copyWith(
                                color: colors.primary,
                                fontWeight: FontWeight.bold,
                                letterSpacing: 3,
                              ),
                        ),
                        const SizedBox(height: 8),
                        Text(
                          '${(_remainingMs / 1000).ceil()}s until next code',
                        ),
                      ],
                    ),
                  ],
                ),
              ),
            ),
            const SizedBox(height: 24),
            FilledButton(
              onPressed: widget.approved ? null : widget.onApprove,
              child: Text(widget.approved ? 'Approved' : 'Send code (demo)'),
            ),
          ],
        ),
      ),
    );
  }
}
